#include <coipc/endpoint_com.h>

#include <atlbase.h>
#include <atlcom.h>

namespace coipc
{
	namespace com
	{
		class session : public ISequentialStream, public IConnectionPoint, public /*outbound*/ coipc::channel,
			public CComObjectRoot
		{
		public:
			BEGIN_COM_MAP(session)
				COM_INTERFACE_ENTRY(ISequentialStream)
				COM_INTERFACE_ENTRY(IConnectionPoint)
			END_COM_MAP()

			void FinalRelease();

		public:
			channel_ptr_t inbound;

		private:
			// ISequentialStream methods
			STDMETHODIMP Read(void *, ULONG, ULONG *);
			STDMETHODIMP Write(const void *message, ULONG size, ULONG *written);

			// IConnectionPoint methods
			STDMETHODIMP GetConnectionInterface(IID *iid);
			STDMETHODIMP GetConnectionPointContainer(IConnectionPointContainer **container);
			STDMETHODIMP Advise(IUnknown *sink, DWORD *cookie);
			STDMETHODIMP Unadvise(DWORD cookie);
			STDMETHODIMP EnumConnections(IEnumConnections **enumerator);

			virtual void disconnect() throw();
			virtual void message(const_byte_range payload);

		private:
			CComPtr<ISequentialStream> _outbound;
		};


		class server : public CComClassFactory
		{
		public:
			void set_server(const std::shared_ptr<coipc::server> &factory);

		private:
			STDMETHODIMP CreateInstance(IUnknown *outer, REFIID riid, void **object);

		private:
			std::shared_ptr<coipc::server> _session_factory;
		};
	}
}
