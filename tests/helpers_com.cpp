#include "helpers_com.h"

#include <atlbase.h>
#include <coipc/com/endpoint.h>
#include <ut/assert.h>

namespace coipc
{
	namespace tests
	{
		namespace
		{
			CComModule g_module;

			class sequential_stream
			{
			public:
				sequential_stream(const CLSID &id)
					: _frontend(0)
				{
					::CoCreateInstance(id, NULL, CLSCTX_LOCAL_SERVER, IID_ISequentialStream, (void **)&_frontend);
					assert_not_null(_frontend);
				}

				sequential_stream(const sequential_stream &other)
					: _frontend(other._frontend)
				{	_frontend->AddRef();	}

				~sequential_stream()
				{	_frontend->Release();	}

				bool operator()(const void *buffer, size_t size)
				{
					ULONG written;

					return S_OK == _frontend->Write(buffer, static_cast<ULONG>(size), &written);
				}

			private:
				ISequentialStream *_frontend;
			};
		}


		com_event::com_event()
			: _handle(::CreateEvent(NULL, FALSE, FALSE, NULL), &::CloseHandle)
		{	}

		void com_event::set()
		{	::SetEvent(_handle.get());	}

		void com_event::wait()
		{
			for (HANDLE h = _handle.get();
				WAIT_OBJECT_0 + 1 == ::MsgWaitForMultipleObjects(1, &h, FALSE, INFINITE, QS_ALLINPUT); )
			{
				MSG msg;

				while (::PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
					::DispatchMessage(&msg);
			}
		}


		bool is_factory_registered(const guid_t &id)
		{
			void *p = 0;

			if (S_OK == ::CoGetClassObject(id, CLSCTX_LOCAL_SERVER, NULL, IID_IClassFactory, &p) && p)
			{
				static_cast<IUnknown *>(p)->Release();
				return true;
			}
			return false;
		}

		stream_function_t open_stream(const guid_t &id)
		{	return sequential_stream(reinterpret_cast<const CLSID &>(id));	}
	}
}
