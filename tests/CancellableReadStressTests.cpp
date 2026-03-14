#include <coipc/cancellable_read.h>

#include "helpers.h"

#include <atomic>
#include <thread>

#include <ut/assert.h>
#include <ut/test.h>

using namespace std;

namespace coipc
{
	namespace tests
	{
		namespace
		{
			void feed_bytes(FILE &file, size_t count)
			{
				for (auto i = 0u; i < count; ++i)
				{
					const char byte = static_cast<char>(i);

					fwrite(&byte, 1, 1, &file);
				}
				fflush(&file);
			}
		}


		begin_test_suite( CancellableReadStressTests )
			test( CancelUnblocksABlockedRead )
			{
				// INIT
				const auto n_iterations = 500u;

				for (auto i = 0u; i < n_iterations; ++i)
				{
					auto pp = create_pipe();
					cancellable_read r;
					atomic<int> cancelled_count(0);
					char buffer[1] = { };

					// ACT
					thread reader([&] {
						try
						{	r.read(*get<0>(pp), buffer, sizeof(buffer));	}
						catch (const cancelled_exception &)
						{	cancelled_count.fetch_add(1, memory_order_relaxed);	}
					});

					r.cancel();
					reader.join();

					// ASSERT
					assert_equal(1, cancelled_count.load());
				}
			}


			test( CancelRacingWithDataArrivalAlwaysTerminates )
			{
				// INIT
				const auto n_iterations = 1000u;
				auto cancelled_count = 0u;
				auto succeeded_count = 0u;

				for (auto i = 0u; i < n_iterations; ++i)
				{
					auto pp = create_pipe();
					cancellable_read r;

					// ACT
					thread reader([&] {
						try
						{
							char buffer[1];
							r.read(*get<0>(pp), buffer, 1);
							succeeded_count++;
						}
						catch (const cancelled_exception &)
						{
							cancelled_count++;
						}
					});

					thread writer([&] {
						feed_bytes(*get<1>(pp), 1);
					});

					this_thread::yield();
					r.cancel();
					reader.join();
					writer.join();
				}

				// ASSERT
				assert_equal(n_iterations, cancelled_count + succeeded_count);
			}


			test( MultipleReadsAreCancelledByASubsequentCancel )
			{
				// INIT
				const auto n_iterations = 300u;
				const auto n_reads_before_cancel = 10u;

				for (auto i = 0u; i < n_iterations; ++i)
				{
					auto pp = create_pipe();
					cancellable_read r;
					auto completed = 0u;
					char buffer[1] = { };

					// ACT
					thread reader([&] {
						for (auto j = 0u; j < n_reads_before_cancel; ++j)
						{
							try
							{
								r.read(*get<0>(pp), buffer, sizeof(buffer));
								completed++;
							}
							catch (const cancelled_exception &)
							{	break;	}
						}
					});

					feed_bytes(*get<1>(pp), n_reads_before_cancel);
					this_thread::yield();
					r.cancel();
					reader.join();

					// ASSERT
					assert_is_true(completed <= n_reads_before_cancel);
					assert_throws(r.read(*get<0>(pp), buffer, sizeof(buffer)), cancelled_exception);
				}
			}


			test( ReadReturnsDataWrittenToPipe )
			{
				// INIT
				const auto n_iterations = 200u;

				for (auto i = 0u; i < n_iterations; ++i)
				{
					auto pp = create_pipe();
					cancellable_read r;
					char buffer[7] = { };

					feed_bytes(*get<1>(pp), sizeof(buffer));

					// ACT
					auto result = r.read(*get<0>(pp), buffer, sizeof(buffer));

					// ASSERT
					assert_is_true(result > 0u);
					assert_is_true(result <= sizeof(buffer));
				}
			}


			test( PostCancelReadAlwaysThrowsEvenWithDataAvailable )
			{
				// INIT
				const auto n_iterations = 500u;

				for (auto i = 0u; i < n_iterations; ++i)
				{
					auto pp = create_pipe();
					cancellable_read r;
					char buffer[4] = { };

					feed_bytes(*get<1>(pp), sizeof(buffer));
					r.cancel();

					// ACT / ASSERT
					assert_throws(r.read(*get<0>(pp), buffer, sizeof(buffer)), cancelled_exception);
				}
			}
		end_test_suite
	}
}