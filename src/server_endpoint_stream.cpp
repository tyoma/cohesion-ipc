#include <coipc/endpoint_stream.h>

#include <coipc/cancellable_read.h>
#include <mt/thread.h>

using namespace std;

namespace coipc
{
	shared_ptr<void> stream::connect(FILE &inbound, FILE &outbound, const session_factory_t &new_server_session)
	{
		class stream_session_connector : public channel
		{
		public:
			stream_session_connector(FILE &inbound, FILE &outbound, const session_factory_t &new_server_session)
				: _outbound(outbound), _worker([this, &inbound, new_server_session] {
					try
					{
						unsigned int size;
						auto session = new_server_session(*this);

						for (vector<uint8_t> buffer; _reader.read(inbound, &size, sizeof(size)); )
						{
							buffer.resize(size);
							_reader.read(inbound, buffer.data(), size);
							session->message(const_byte_range(buffer.data(), size));
						}
					}
					catch (...)
					{
					}
				})
			{	}

			~stream_session_connector()
			{
				_reader.cancel();
				_worker.join();
			}

			virtual void disconnect() throw()
			{	}

			virtual void message(const_byte_range payload)
			{	}

		private:
			FILE &_outbound;
			cancellable_read _reader;
			mt::thread _worker;
		};

		return make_shared<stream_session_connector>(inbound, outbound, new_server_session);
	}
}
