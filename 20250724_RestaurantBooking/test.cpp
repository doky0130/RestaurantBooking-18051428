#include "gmock/gmock.h"
#include "booking_scheduler.cpp"

class BookingSchedulerTest : public testing::Test {
public:
	Customer customer{ "name","010-1234-5678" };
	tm notOnTheHour;
	tm onTheHour;

	void setUp() {
		notOnTheHour = getTime(2025, 7, 24, 12, 56);
		onTheHour = getTime(2025, 7, 24, 13, 0);
	}

	tm getTime(int year, int month, int day, int hour, int min) {
		tm result = { 0, min, hour, day, month - 1, year - 1900, 0, 0, -1};
		mktime(&result);
		return result;
	}
};
TEST_F(BookingSchedulerTest, 예약은정시에만가능하다정시가아닌경우예약불가) {
	setUp();

	Schedule* schedule = new Schedule{ notOnTheHour , 1, customer };
	BookingScheduler bookingScheduler{ 3 };

	EXPECT_THROW(bookingScheduler.addSchedule(schedule),
		std::runtime_error);
}

TEST_F(BookingSchedulerTest, 예약은정시에만가능하다정시인경우예약가능) {
	setUp();

	Customer customer{ "name","010-1234-5678" };
	Schedule* schedule = new Schedule{ onTheHour , 1, customer };
	BookingScheduler bookingScheduler{ 3 };

	bookingScheduler.addSchedule(schedule);
	EXPECT_EQ(true, bookingScheduler.hasSchedule(schedule));
}

TEST_F(BookingSchedulerTest, 시간대별인원제한이있다같은시간대에Capacity초과할경우예외발생) {

}

TEST_F(BookingSchedulerTest, 시간대별인원제한이있다같은시간대가다르면Capacity차있어도스케쥴추가성공) {

}

TEST_F(BookingSchedulerTest, 예약완료시SMS는무조건발송) {

}

TEST_F(BookingSchedulerTest, 이메일이없는경우에는이메일미발송) {

}

TEST_F(BookingSchedulerTest, 이메일이있는경우에는이메일발송) {

}

TEST_F(BookingSchedulerTest, 현재날짜가일요일인경우예약불가예외처리) {

}

TEST_F(BookingSchedulerTest, 현재날짜가일요일이아닌경우예약가능) {

}

int main() {
	::testing::InitGoogleMock();
	return RUN_ALL_TESTS();
}