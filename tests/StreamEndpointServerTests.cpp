#include <coipc/endpoint_stream.h>

#include "helpers.h"
#include "mocks.h"

#include <mt/event.h>
#include <mt/thread.h>
#include <ut/assert.h>
#include <ut/test.h>

using namespace std;

namespace coipc
{
	namespace tests
	{
		begin_test_suite( AStreamEndpointServerTests )
			test( NewSessionUnconditionallyGetsCreatedInNewThreadAndThenDestroyedUponServerRelease )
			{
				// INIT
				mt::event session_created_event, session_destroyed_event;
				auto pipe1 = create_pipe(); // server reads from pipe1
				auto pipe2 = create_pipe(); // server writes to pipe2
				shared_ptr<mocks::session> created_session;
				mt::thread::id server_thread_id;

				// ACT
				auto h = stream::connect(*get<0>(pipe1), *get<1>(pipe2), [&] (channel &/*outbound*/) {
					created_session = make_shared<mocks::session>();
					server_thread_id = mt::this_thread::get_id();
					session_created_event.set();
					return created_session;
				});

				// ASSERT
				session_created_event.wait();
				assert_not_null(created_session);
				assert_not_equal(mt::this_thread::get_id(), server_thread_id);

				// ACT
				h.reset();

				// ASSERT
				assert_equal(1, created_session.use_count());
			}


			test( MessagesFromInputStreamAreDeliveredToServerSession )
			{
				// INIT
				mt::event session_created, message_received;
				auto inbound = create_pipe(); // server reads from pipe1
				auto outbound = create_pipe(); // server writes to pipe2
				shared_ptr<mocks::session> created_session;
				auto h = stream::connect(*get<0>(inbound), *get<1>(outbound), [&] (channel &/*outbound*/) {
					created_session = make_shared<mocks::session>();
					session_created.set();
					return created_session;
				});
				byte payload1[] = { 13, 1, };
				byte payload2[] = { 7, 9, 11, 19, 2, 7 };
				byte payload3[50000] = { 7, 9, 0, 1, 2, 7, };
				unsigned int size = sizeof payload1;

				session_created.wait();
				created_session->received_message = [&] {	message_received.set();	};

				// ACT
				fwrite(&size, sizeof size, 1, get<1>(inbound).get());
				fwrite(payload1, 1, sizeof payload1, get<1>(inbound).get());
				fflush(get<1>(inbound).get());
				message_received.wait();

				// ASSERT
				assert_equal(1u, created_session->payloads_log.size());
				assert_equal(payload1, created_session->payloads_log.back());
				
				// INIT
				size = sizeof payload2;
				
				// ACT
				fwrite(&size, sizeof size, 1, get<1>(inbound).get());
				fwrite(payload2, 1, sizeof payload2, get<1>(inbound).get());
				fflush(get<1>(inbound).get());
				message_received.wait();
				
				// ASSERT
				assert_equal(2u, created_session->payloads_log.size());
				assert_equal(payload2, created_session->payloads_log.back());
				
				// INIT
				size = sizeof payload3;
				
				// ACT
				fwrite(&size, sizeof size, 1, get<1>(inbound).get());
				fwrite(payload3, 1, sizeof payload3, get<1>(inbound).get());
				fflush(get<1>(inbound).get());
				message_received.wait();
				
				// ASSERT
				assert_equal(3u, created_session->payloads_log.size());
				assert_equal(payload3, created_session->payloads_log.back());
			}

		end_test_suite
	}
}
