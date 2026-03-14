#include <coipc/cancellable_read.h>

#include "fake_read_primitives.h"

#include <ut/assert.h>
#include <ut/test.h>

using namespace std;

namespace coipc
{
	namespace tests
	{
		begin_test_suite( CancellableReadTests )
			test( CancelIsStickyRejectsFurtherReadsImmediately )
			{
				// INIT
				fake_read_primitives p;
				cancellable_read r(p);
				char buffer[4] = { };

				r.cancel();

				// ACT / ASSERT
				assert_throws(r.read(*reinterpret_cast<FILE *>(1), buffer, sizeof(buffer)), cancelled_exception);

				// ASSERT
				assert_equal(0u, p.read_calls());
			}


			test( CancelIsIdempotent )
			{
				// INIT
				fake_read_primitives p;
				cancellable_read r(p);
				char buffer[1] = { };

				r.cancel();
				r.cancel();
				r.cancel();

				// ACT / ASSERT
				assert_throws(r.read(*reinterpret_cast<FILE *>(1), buffer, sizeof(buffer)), cancelled_exception);

				// ASSERT
				assert_equal(0u, p.read_calls());
			}


			test( CancelDuringReadCausesReadToThrowCancelled )
			{
				// INIT
				fake_read_primitives p;
				cancellable_read r(p);
				char buffer[8] = { };

				p.block_next_read();

				thread t([&] {
					assert_throws(r.read(*reinterpret_cast<FILE *>(1), buffer, sizeof(buffer)), cancelled_exception);
				});

				p.wait_until_read_entered();

				// ACT
				r.cancel();

				p.unblock_read();
				t.join();
			}


			test( CancelRaceAfterPrecheckBeforeReadEngagesIsStillCancelled )
			{
				// INIT
				fake_read_primitives p;
				cancellable_read r(p);
				char buffer[8] = { };

				p.pause_before_engage();

				thread t([&] {
					assert_throws(r.read(*reinterpret_cast<FILE *>(1), buffer, sizeof(buffer)), cancelled_exception);
				});

				p.wait_until_about_to_engage();

				// ACT
				r.cancel();

				p.allow_engage();
				t.join();
			}


			test( CancelRetriesUntilCancelSyncIoReportsSuccess )
			{
				// INIT
				fake_read_primitives p;
				cancellable_read r(p);
				char buffer[8] = { };

				p.block_next_read();
				p.set_cancel_plan(/*failures_before_success*/ 5);

				thread t([&] {
					assert_throws(r.read(*reinterpret_cast<FILE *>(1), buffer, sizeof(buffer)), cancelled_exception);
				});

				p.wait_until_read_entered();

				// ACT
				r.cancel();
				p.unblock_read();
				t.join();

				// ASSERT
				assert_equal(6u, p.cancel_calls()); // 5x false + 1x true
			}


			test( CancelAfterReadCompletedMakesFurtherReadsFail )
			{
				// INIT
				fake_read_primitives p;
				cancellable_read r(p);
				char buffer[8] = { };

				r.read(*reinterpret_cast<FILE *>(1), buffer, sizeof(buffer));

				// ACT
				r.cancel();

				// ACT / ASSERT
				assert_throws(r.read(*reinterpret_cast<FILE *>(1), buffer, sizeof(buffer)), cancelled_exception);
			}


			test( CancelWithNoReadDoesNotSpin )
			{
				// INIT
				fake_read_primitives p;
				cancellable_read r(p);

				// ACT
				r.cancel();

				// ASSERT
				assert_equal(0u, p.cancel_calls());
			}


			test( ReadReturnsByteCountFromPlatformRead )
			{
				// INIT
				fake_read_primitives p;
				cancellable_read r(p);
				char buffer[8] = { };

				// ACT
				size_t result1 = r.read(*reinterpret_cast<FILE *>(1), buffer, sizeof(buffer));
				size_t result2 = r.read(*reinterpret_cast<FILE *>(1), buffer, 0);

				// ASSERT
				assert_equal(1u, result1); // fake_read_primitives returns min(size, 1)
				assert_equal(0u, result2);
			}
		end_test_suite
	}
}