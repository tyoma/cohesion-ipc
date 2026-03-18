#include <coipc/endpoint_stream.h>

#include "helpers.h"
#include "mocks.h"

#include <list>
#include <mt/event.h>
#include <mt/thread.h>
#include <ut/assert.h>
#include <ut/test.h>

using namespace std;

namespace coipc
{
	namespace tests
	{
		begin_test_suite( StreamEndpointServerTests )
			mt::event ready;
			list<mt::thread> threads_to_join;

			template <typename T>
			void run_thread(const T &thead_function)
			{	threads_to_join.emplace_back(thead_function);	}

			teardown( JoinThreads )
			{
				for (auto &t : threads_to_join)
					t.join();
			}

			test( NewSessionUnconditionallyGetsCreatedInNewThreadAndThenDestroyedUponServerRelease )
			{
				// INIT
				auto inbound = create_pipe(); // server reads from pipe1
				auto outbound = create_pipe(); // server writes to pipe2
				shared_ptr<mocks::session> created_session;
				mt::thread::id server_thread_id;

				// ACT
				auto h = stream::connect(*get<0>(inbound), *get<1>(outbound), [&] (channel &/*outbound*/) {
					created_session = make_shared<mocks::session>();
					server_thread_id = mt::this_thread::get_id();
					ready.set();
					return created_session;
				});

				// ASSERT
				ready.wait();
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
				auto inbound = create_pipe(); // server reads from pipe1
				auto outbound = create_pipe(); // server writes to pipe2
				shared_ptr<mocks::session> created_session;
				auto h = stream::connect(*get<0>(inbound), *get<1>(outbound), [&] (channel &/*outbound*/) {
					created_session = make_shared<mocks::session>();
					ready.set();
					return created_session;
				});
				byte payload1[] = { 13, 1, };
				byte payload2[] = { 7, 9, 11, 19, 2, 7 };
				byte payload3[50000] = { 7, 9, 0, 1, 2, 7, };
				unsigned int size = sizeof payload1;

				ready.wait();
				created_session->received_message = [&] {	ready.set();	};

				// ACT
				fwrite(&size, sizeof size, 1, get<1>(inbound).get());
				fwrite(payload1, 1, sizeof payload1, get<1>(inbound).get());
				fflush(get<1>(inbound).get());
				ready.wait();

				// ASSERT
				assert_equal(1u, created_session->payloads_log.size());
				assert_equal(payload1, created_session->payloads_log.back());
				
				// INIT
				size = sizeof payload2;
				
				// ACT
				fwrite(&size, sizeof size, 1, get<1>(inbound).get());
				fwrite(payload2, 1, sizeof payload2, get<1>(inbound).get());
				fflush(get<1>(inbound).get());
				ready.wait();
				
				// ASSERT
				assert_equal(2u, created_session->payloads_log.size());
				assert_equal(payload2, created_session->payloads_log.back());
				
				// INIT
				size = sizeof payload3;
				
				// ACT
				fwrite(&size, sizeof size, 1, get<1>(inbound).get());
				fwrite(payload3, 1, sizeof payload3, get<1>(inbound).get());
				fflush(get<1>(inbound).get());
				ready.wait();
				
				// ASSERT
				assert_equal(3u, created_session->payloads_log.size());
				assert_equal(payload3, created_session->payloads_log.back());
			}


			test( OutboundMessagesAreSentAsExpected )
			{
				// INIT
				auto inbound = create_pipe(); // server reads from pipe1
				auto outbound = create_pipe(); // server writes to pipe2
				shared_ptr<mocks::session> session;
				auto h = stream::connect(*get<0>(inbound), *get<1>(outbound), [&] (channel &outbound) {
					session = make_shared<mocks::session>();
					session->outbound = &outbound;
					ready.set();
					return session;
				});
				byte payload1[] = { 13, 1, };
				byte payload2[] = { 7, 9, 11, 19, 2, 7 };
				byte payload3[50000] = { 7, 9, 0, 1, 2, 7, };
				unsigned int bytes_to_read;
				vector<byte> buffer_to_read;

				ready.wait();

				// ACT
				run_thread([&] {	session->outbound->message(const_byte_range(payload1, sizeof payload1));	});

				// ASSERT
				fread(&bytes_to_read, sizeof bytes_to_read, 1, get<0>(outbound).get());
				assert_equal(sizeof payload1, bytes_to_read);
				buffer_to_read.resize(bytes_to_read);
				fread(buffer_to_read.data(), bytes_to_read, 1, get<0>(outbound).get());
				assert_equal(payload1, buffer_to_read);

				// ACT
				run_thread([&] {	session->outbound->message(const_byte_range(payload2, sizeof payload2));	});
				
				// ASSERT
				fread(&bytes_to_read, sizeof bytes_to_read, 1, get<0>(outbound).get());
				assert_equal(sizeof payload2, bytes_to_read);
				buffer_to_read.resize(bytes_to_read);
				fread(buffer_to_read.data(), bytes_to_read, 1, get<0>(outbound).get());
				assert_equal(payload2, buffer_to_read);

				// ACT
				run_thread([&] {	session->outbound->message(const_byte_range(payload3, sizeof payload3));	});

				// ASSERT
				fread(&bytes_to_read, sizeof bytes_to_read, 1, get<0>(outbound).get());
				assert_equal(sizeof payload3, bytes_to_read);
				buffer_to_read.resize(bytes_to_read);
				fread(buffer_to_read.data(), bytes_to_read, 1, get<0>(outbound).get());
				assert_equal(payload3, buffer_to_read);
			}


			test( SessionReceivesDisconnectWhenWriterSideOfInboundPipeIsClosed )
			{
				// INIT
				auto inbound = create_pipe(); // server reads from pipe1
				auto outbound = create_pipe(); // server writes to pipe2
				shared_ptr<mocks::session> session;
				auto h = stream::connect(*get<0>(inbound), *get<1>(outbound), [&] (channel &/*outbound*/) {
					session = make_shared<mocks::session>();
					session->disconnected = [&] {	ready.set();	};
					ready.set();
					return session;
				});

				ready.wait();

				// ACT
				get<1>(inbound).reset();

				// ACT / ASSERT
				ready.wait();
			}


			test( SessionDoesNotReceiveDisconnectWhenServerSessionIsStoppedLocally )
			{
				// INIT
				auto inbound = create_pipe(); // server reads from pipe1
				auto outbound = create_pipe(); // server writes to pipe2
				shared_ptr<mocks::session> session;
				auto h = stream::connect(*get<0>(inbound), *get<1>(outbound), [&] (channel &/*outbound*/) {
					session = make_shared<mocks::session>();
					session->disconnected = [&] {	ready.set();	};
					ready.set();
					return session;
				});

				ready.wait();

				// ACT
				h.reset();

				// ASSERT
				assert_equal(0u, session->disconnections);
			}

		end_test_suite
	}
}
