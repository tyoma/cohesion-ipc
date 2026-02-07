#include <coipc/com/endpoint.h>

#include <coipc/misc.h>
#include <coipc/range.h>

#include <functional>

using namespace std;

namespace coipc
{
	namespace com
	{
		namespace
		{
			template <typename T>
			CComPtr< CComObject<T> > construct()
			{
				CComObject<T> *p;

				if (E_OUTOFMEMORY == CComObject<T>::CreateInstance(&p) || !p)
					throw bad_alloc();
				return CComPtr< CComObject<T> > (p);
			}
		}



		void session::FinalRelease()
		{	inbound->disconnect();	}

		STDMETHODIMP session::Read(void *, ULONG, ULONG *)
		{	return E_UNEXPECTED;	}

		STDMETHODIMP session::Write(const void *message, ULONG size, ULONG * /*written*/)
		{
			inbound->message(const_byte_range(static_cast<const uint8_t *>(message), size));
			return S_OK;
		}

		STDMETHODIMP session::GetConnectionInterface(IID *iid)
		{	return iid ? *iid = IID_ISequentialStream, S_OK : E_POINTER;	}

		STDMETHODIMP session::GetConnectionPointContainer(IConnectionPointContainer ** /*container*/)
		{	return E_NOTIMPL;	}

		STDMETHODIMP session::Advise(IUnknown *sink, DWORD *cookie)
		{
			if (!cookie)
				return E_POINTER;
			if (!sink)
				return E_INVALIDARG;
			if (_outbound)
				return CONNECT_E_ADVISELIMIT;

			HRESULT hr = sink->QueryInterface(&_outbound);

			if (S_OK == hr)
			{
				*cookie = 1;
			}
			return hr;
		}

		STDMETHODIMP session::Unadvise(DWORD cookie)
		{	return cookie == 1 && _outbound ? _outbound = 0, S_OK : CONNECT_E_NOCONNECTION;	}

		STDMETHODIMP session::EnumConnections(IEnumConnections ** /*enumerator*/)
		{	return E_NOTIMPL;	}

		void session::disconnect()
		{	::CoDisconnectObject(static_cast<ISequentialStream *>(this), 0);	}

		void session::message(const_byte_range payload)
		{
			if (!_outbound)
				throw runtime_error("Sink channel has not been created yet!");
			_outbound->Write(payload.begin(), static_cast<ULONG>(payload.length()), NULL);
		}


		void server::set_server(const shared_ptr<coipc::server> &factory)
		{	_session_factory = factory;	}

		STDMETHODIMP server::CreateInstance(IUnknown * /*outer*/, REFIID riid, void **object)
		try
		{
			CComPtr< CComObject<session> > p = construct<session>();

			p->inbound = _session_factory->create_session(*p);
			return p->QueryInterface(riid, object);
		}
		catch (const bad_alloc &)
		{
			return E_OUTOFMEMORY;
		}


		shared_ptr<void> run_server(const char *endpoint_id, const shared_ptr<coipc::server> &sf)
		{
			const auto id = guid_from_string(endpoint_id);
			const auto p = construct<server>();
			DWORD cookie;

			p->set_server(sf);
			if (S_OK == ::CoRegisterClassObject(id, p, CLSCTX_LOCAL_SERVER, REGCLS_MULTIPLEUSE, &cookie))
				return shared_ptr<void>(static_cast<IUnknown *>(p), bind(&::CoRevokeClassObject, cookie));
			throw initialization_failed(endpoint_id);
		}
	}
}
