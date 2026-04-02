#include <coipc/endpoint_spawn.h>

#include "constants.h"
#include "helpers.h"
#include "mocks.h"
#include "path.h"
#include "time.h"

#include <coipc/exceptions.h>
#include <coipc/types.h>
#include <mt/event.h>
#include <ut/assert.h>
#include <ut/test.h>

#pragma warning(disable: 4244)

using namespace std;

namespace coipc
{
	namespace tests
	{
		bool is_process_running(unsigned int pid);

		namespace
		{
			const vector<string> no_args;
			const vector<string> no_extra;

			inline void operator <<(channel &lhs, const vector<byte> &rhs)
			{	lhs.message(const_byte_range(rhs.data(), rhs.size()));	}

			void ignore_exit(int)
			{	}
		}

		begin_test_suite( SpawnEndpointClientTests )
			mt::event ready;
			mocks::channel inbound;


			test( AttemptToSpawnAMissingFileThrows )
			{
#ifdef _WIN32
				// INIT / ACT / ASSERT
				assert_throws(spawn::connect_client("zubazuba", no_args, no_extra, inbound, ignore_exit),
					server_exe_not_found);
				assert_throws(spawn::connect_client(~constants::c_this_module & normalize::exe("abc\\guinea_ipc_spawn"),
					no_args, no_extra, inbound, ignore_exit), server_exe_not_found);
#else
				// INIT
				inbound.on_disconnect = [&] {	ready.set();	};

				// INIT / ACT
				auto c1 = spawn::connect_client("zubazuba", no_args, no_extra, inbound, ignore_exit);

				// ACT
				ready.wait();

				// INIT / ACT
				auto c2 = spawn::connect_client(~constants::c_this_module & normalize::exe("abc\\guinea_ipc_spawn"),
					no_args, no_extra, inbound, ignore_exit);

				// ACT
				ready.wait();
#endif
			}


			test( SpawningAnExistingExecutableReturnsNonNullChannelAndNotifiesAboutImmediateDisconnection )
			{
				// INIT
				inbound.on_disconnect = [&] {	ready.set();	};

				// INIT / ACT
				auto outbound = spawn::connect_client(constants::c_guinea_ipc_spawn, no_args, no_extra, inbound,
					ignore_exit);

				// ASSERT
				assert_not_null(outbound);

				// ACT / ASSERT
				ready.wait();
			}


			test( DisconnectionIsNotSentUntilTheServerExits )
			{
				// INIT
				stopwatch sw;
				auto duration = 0.0;

				inbound.on_disconnect = [&] {	duration = sw(), ready.set();	};

				// INIT / ACT
				auto outbound = spawn::connect_client(constants::c_guinea_ipc_spawn,
					plural + (string)"sleep" + (string)"100", no_extra, inbound, ignore_exit);
				sw(); // Reset timer.

				// ACT
				ready.wait();

				// ASSERT
				assert_is_true(0.1 <= duration);
				assert_is_true(0.3 > duration);

				// INIT / ACT
				outbound = spawn::connect_client(constants::c_guinea_ipc_spawn,
					plural + (string)"sleep" + (string)"300", no_extra, inbound, ignore_exit);
				sw(); // Reset timer.

				// ACT
				ready.wait();

				// ASSERT
				assert_is_true(0.3 <= duration);
				assert_is_true(0.6 > duration);
			}


			test( IncomingMessagesAreDeliveredToInboundChannel )
			{
				// INIT
				vector<string> messages;

				inbound.on_message = [&] (const_byte_range payload) {
					messages.push_back(string(payload.begin(), payload.end()));
					if (3u == messages.size())
						ready.set();
				};

				// INIT / ACT
				auto outbound = spawn::connect_client(constants::c_guinea_ipc_spawn,
					plural + (string)"seq" + (string)"Lorem" + (string)"ipsum" + (string)"amet dolor", no_extra,
					inbound, ignore_exit);

				// ACT
				ready.wait();

				// ASSERT
				assert_equal(plural + (string)"Lorem" + (string)"ipsum" + (string)"amet dolor", messages);

				// INIT
				messages.clear();
				inbound.on_message = [&] (const_byte_range payload) {
					messages.push_back(string(payload.begin(), payload.end()));
					if (2u == messages.size())
						ready.set();
				};

				// INIT / ACT
				outbound = spawn::connect_client(constants::c_guinea_ipc_spawn,
					plural + (string)"seq" + (string)"Quick brown fox" + (string)"jumps over the\nlazy dog", no_extra,
					inbound, ignore_exit);

				// ACT
				ready.wait();

				// ASSERT
				assert_equal(plural + (string)"Quick brown fox" + (string)"jumps over the\nlazy dog", messages);
			}


			test( OutboundMessagesAreDeliveredToTheServer )
			{
				// INIT
				vector< vector<byte> > messages;
				auto disconnected = false;
				vector<byte> data1(15), data2(1192311);
				vector<byte> read;

				generate(data1.begin(), data1.end(), rand);
				generate(data2.begin(), data2.end(), rand);

				inbound.on_disconnect = [&] {
					disconnected = true;
					ready.set();
				};
				inbound.on_message = [&] (const_byte_range payload) {
					messages.push_back(vector<byte>(payload.begin(), payload.end()));
					ready.set();
				};

				auto outbound = spawn::connect_client(constants::c_guinea_ipc_spawn, plural + (string)"echo", no_extra,
					inbound, ignore_exit);

				// ACT
				*outbound << data1;
				ready.wait();

				// ASSERT
				assert_equal(plural + data1, messages);

				// ACT
				*outbound << data2;
				ready.wait();

				// ASSERT
				assert_equal(plural + data1 + data2, messages);
				assert_is_false(disconnected);
					
				// ACT
				*outbound << vector<byte>();
				ready.wait();

				// ASSERT
				assert_is_true(disconnected);
			}


			test( ClientCanBeDestroyedWhenAServerIsPendingForInput )
			{
				// INIT
				auto disconnected = false;
				auto outbound = spawn::connect_client(constants::c_guinea_ipc_spawn, plural + (string)"echo", no_extra,
					inbound, ignore_exit);

				inbound.on_disconnect = [&] {	disconnected = true;	};

				// ACT / ASSERT (does not hang)
				outbound.reset();

				// ASSERT
				assert_is_false(disconnected);
			}


			test( ApplicationIsStartedWithExtraEnvironmentSpecified )
			{
				// INIT
				vector<string> environment;

				inbound.on_message = [&] (const_byte_range payload) {
					if (payload.length())
						environment.push_back(string(payload.begin(), payload.end()));
					else
						ready.set();
				};

				// ACT
				auto client = spawn::connect_client(constants::c_guinea_ipc_spawn, plural + (string)"environment",
					no_extra, inbound, ignore_exit);
				ready.wait();

				auto e1 = std::move(environment);

				client = spawn::connect_client(constants::c_guinea_ipc_spawn, plural + (string)"environment",
					plural + (string)"foo=foobar" + (string)"lorem=lorem ipsum amet dolor", inbound, ignore_exit);
				ready.wait();

				auto e2 = std::move(environment);

				client = spawn::connect_client(constants::c_guinea_ipc_spawn, plural + (string)"environment",
					plural + (string)"foo=bazbar" + (string)"lorem=dolor" + (string)"algo algo=alco alco", inbound,
					ignore_exit);
				ready.wait();

				auto e3 = std::move(environment);

				// ASSERT
				sort(e1.begin(), e1.end());
				sort(e2.begin(), e2.end());
				sort(e3.begin(), e3.end());

				set_difference(e2.begin(), e2.end(), e1.begin(), e1.end(), back_inserter(environment));
				assert_equivalent(plural + (string)"foo=foobar" + (string)"lorem=lorem ipsum amet dolor", environment);
				environment.clear();
				set_difference(e3.begin(), e3.end(), e1.begin(), e1.end(), back_inserter(environment));
				assert_equivalent(plural + (string)"foo=bazbar" + (string)"lorem=dolor" + (string)"algo algo=alco alco", environment);
			}


			test( SpawnedProcessIsRunningPriorDestructionAndNeverAfter )
			{
				for (auto n = 100; n--; )
				{
					// INIT
					auto pid = 0u;

					inbound.on_message = [&] (const_byte_range payload) {
						pid = *reinterpret_cast<const unsigned *>(payload.data());
						ready.set();
					};

					// ACT
					auto client = spawn::connect_client(constants::c_guinea_ipc_spawn, plural + (string)"pid", no_extra,
						inbound, ignore_exit);
					ready.wait();

					// ASSERT
					assert_is_true(coipc::tests::is_process_running(pid));

					// ACT
					client.reset();

					// ASSERT
					assert_is_false(coipc::tests::is_process_running(pid));
				}
			}


			test( ExitHandlerIsInvokedWithAnExitCodeFromSpawnedProcess )
			{
				// INIT
				auto exit_code = -1;

				// ACT
				auto client = spawn::connect_client(constants::c_guinea_ipc_spawn,
					plural + (string)"exit" + (string)"123", no_extra, inbound, [&] (int code) {

					exit_code = code;
					ready.set();
				});
				ready.wait();

				// ASSERT
				assert_equal(123, exit_code);

				// ACT
				client = spawn::connect_client(constants::c_guinea_ipc_spawn,
					plural + (string)"exit" + (string)"171", no_extra, inbound, [&] (int code) {

					exit_code = code;
					ready.set();
				});
				ready.wait();

				// ASSERT
				assert_equal(171, exit_code);
			}
		end_test_suite
	}
}
