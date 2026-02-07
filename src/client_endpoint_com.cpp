#include <coipc/endpoint.h>

#include <atlbase.h>
#include <coipc/com/init.h>
#include <coipc/misc.h>
#include <functional>

using namespace std;

namespace coipc
{
	namespace com
	{
		class inbound_stream : public ISequentialStream
		{
		public:
			inbound_stream(channel &underlying);
			~inbound_stream();

			// IUnknown methods
			STDMETHODIMP QueryInterface(REFIID riid, void **ppvObject);
			STDMETHODIMP_(ULONG) AddRef();
			STDMETHODIMP_(ULONG) Release();

			// ISequentialStream methods
			STDMETHODIMP Read(void *, ULONG, ULONG *);
			STDMETHODIMP Write(const void *message, ULONG size, ULONG *written);

		private:
			unsigned _references;
			channel &_underlying;
		};

		class client_session : public channel
		{
		public:
			client_session(const char *destination_endpoint_id, channel &inbound);

			virtual void disconnect() throw();
			virtual void message(const_byte_range payload);

		private:
			static shared_ptr<void> create_activation_context();
			static shared_ptr<void> lock_activation_context(const shared_ptr<void> &ctx);

		private:
			channel &_inbound;
			com_initialize _com_initialize;
			CComPtr<ISequentialStream> _stream;
			DWORD _sink_cookie;
		};



		inbound_stream::inbound_stream(channel &underlying)
			: _references(0), _underlying(underlying)
		{	}

		inbound_stream::~inbound_stream()
		{	_underlying.disconnect();	}

		STDMETHODIMP inbound_stream::QueryInterface(REFIID riid, void **object)
		{
			if (IID_IUnknown != riid && IID_ISequentialStream != riid)
				return E_NOINTERFACE;
			*object = (ISequentialStream *)this;
			AddRef();
			return S_OK;
		}

		STDMETHODIMP_(ULONG) inbound_stream::AddRef()
		{	return ++_references;	}

		STDMETHODIMP_(ULONG) inbound_stream::Release()
		{
			unsigned int r = --_references;

			if (!r)
				delete this;
			return r;
		}

		STDMETHODIMP inbound_stream::Read(void *, ULONG, ULONG *)
		{	return E_NOTIMPL;	}

		STDMETHODIMP inbound_stream::Write(const void *buffer, ULONG size, ULONG * /*written*/)
		{	return _underlying.message(const_byte_range(static_cast<const uint8_t *>(buffer), size)), S_OK;	}



		client_session::client_session(const char *destination_endpoint_id, channel &inbound)
			: _inbound(inbound)
		{
			shared_ptr<void> ctx = create_activation_context();
			shared_ptr<void> ctx_lock = lock_activation_context(ctx);
			const guid_t id = guid_from_string(destination_endpoint_id);
			CComPtr<inbound_stream> sink(new inbound_stream(inbound));

			if (S_OK != _stream.CoCreateInstance(id, NULL, CLSCTX_LOCAL_SERVER))
				throw connection_refused(destination_endpoint_id);

			CComQIPtr<IConnectionPoint>(_stream)->Advise(sink, &_sink_cookie);
		}

		void client_session::disconnect() throw()
		{	}

		void client_session::message(const_byte_range payload)
		{
			ULONG written;
			_stream->Write(payload.begin(), static_cast<ULONG>(payload.length()), &written);
		}

		shared_ptr<void> client_session::create_activation_context()
		{
			ACTCTX ctx = { sizeof(ACTCTX), };

			::GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCTSTR)(void *)&create_activation_context,
				&ctx.hModule);
			ctx.lpResourceName = ISOLATIONAWARE_MANIFEST_RESOURCE_ID;
			ctx.dwFlags = ACTCTX_FLAG_HMODULE_VALID | ACTCTX_FLAG_RESOURCE_NAME_VALID;

			HANDLE hcontext = ::CreateActCtx(&ctx);

			return INVALID_HANDLE_VALUE == hcontext
				? shared_ptr<void>()
				: shared_ptr<void>(hcontext, &::ReleaseActCtx);
		}

		shared_ptr<void> client_session::lock_activation_context(const shared_ptr<void> &ctx)
		{
			ULONG_PTR cookie;

			return ctx && ::ActivateActCtx(ctx.get(), &cookie)
				? shared_ptr<void>(reinterpret_cast<void*>(cookie), bind(&::DeactivateActCtx, 0, cookie))
				: shared_ptr<void>();
		}


		channel_ptr_t connect_client(const char *destination_endpoint_id, channel &inbound)
		{	return channel_ptr_t(new client_session(destination_endpoint_id, inbound));	}
	}
}
