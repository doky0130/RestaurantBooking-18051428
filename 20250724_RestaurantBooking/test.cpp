#include "gmock/gmock.h"
#include "booking_scheduler.cpp"
#include "test_day_of_week_booking_scheduler.cpp"

using namespace testing;

class MockCustomer : public Customer {
public:
	MOCK_METHOD(string, getEmail, (), (override));
};

class MockSmsSender : public SmsSender {
public:
	MOCK_METHOD(void, send, (Schedule*), (override));
};

class MockMailSender : public MailSender {
public:
	MOCK_METHOD(void, sendMail, (Schedule*), (override));
};

class BookingSchedulerTest : public Test {
public:
	MockCustomer CUSTOMER;
	MockCustomer CUSTOMER_WITH_MAIL;
	tm NOT_ON_THE_HOUR;
	tm ON_THE_HOUR;
	const int UNDER_CAPACITY = 1;
	const int CAPACITY_PER_HOUR = 3;
	Schedule* schedule;
	BookingScheduler bookingScheduler{ CAPACITY_PER_HOUR };
	NiceMock<MockSmsSender> smsSender;
	NiceMock <MockMailSender> mailSender;

	void setUp() {
		EXPECT_CALL(CUSTOMER, getEmail)
			.WillRepeatedly(testing::Return(""));
		EXPECT_CALL(CUSTOMER_WITH_MAIL, getEmail)
			.WillRepeatedly(testing::Return("mail@mail.com"));

		NOT_ON_THE_HOUR = getTime(2025, 7, 24, 12, 56);
		ON_THE_HOUR = getTime(2025, 7, 24, 13, 0);

		bookingScheduler.setSmsSender(&smsSender);
		bookingScheduler.setMailSender(&mailSender);
	}

	tm getTime(int year, int month, int day, int hour, int min) {
		tm result = { 0, min, hour, day, month - 1, year - 1900, 0, 0, -1};
		mktime(&result);
		return result;
	}

	tm plusHour(tm orgHour) {
		tm resultHour = orgHour;
		resultHour.tm_hour += 1;
		return resultHour;
	}
};
TEST_F(BookingSchedulerTest, 예약은정시에만가능하다정시가아닌경우예약불가) {
	setUp();
	EXPECT_CALL(smsSender, send).Times(0);

	Schedule* schedule = new Schedule{ NOT_ON_THE_HOUR , UNDER_CAPACITY, CUSTOMER };

	EXPECT_THROW(bookingScheduler.addSchedule(schedule),
		std::runtime_error);
}

TEST_F(BookingSchedulerTest, 예약은정시에만가능하다정시인경우예약가능) {
	setUp();

	Schedule* schedule = new Schedule{ ON_THE_HOUR , CAPACITY_PER_HOUR, CUSTOMER };

	bookingScheduler.addSchedule(schedule);
	EXPECT_EQ(true, bookingScheduler.hasSchedule(schedule));
}

TEST_F(BookingSchedulerTest, 시간대별인원제한이있다같은시간대에Capacity초과할경우예외발생) {
	setUp();

	Schedule* schedule = new Schedule{ ON_THE_HOUR , CAPACITY_PER_HOUR, CUSTOMER };
	bookingScheduler.addSchedule(schedule);

	try {
		schedule = new Schedule{ ON_THE_HOUR , UNDER_CAPACITY, CUSTOMER };
		bookingScheduler.addSchedule(schedule);
		FAIL();
	}
	catch (std::exception& e) {
		EXPECT_EQ(string{ e.what() }, "Number of people is over restaurant capacity per hour");
	}
}

TEST_F(BookingSchedulerTest, 시간대별인원제한이있다같은시간대가다르면Capacity차있어도스케쥴추가성공) {
	setUp();
	EXPECT_CALL(smsSender, send).Times(2);

	Schedule* schedule = new Schedule{ ON_THE_HOUR , CAPACITY_PER_HOUR, CUSTOMER };
	bookingScheduler.addSchedule(schedule);

	tm differentHour = plusHour(ON_THE_HOUR);
	schedule = new Schedule{ differentHour , CAPACITY_PER_HOUR, CUSTOMER };

	bookingScheduler.addSchedule(schedule);
	EXPECT_EQ(true, bookingScheduler.hasSchedule(schedule));
}

TEST_F(BookingSchedulerTest, 예약완료시SMS는무조건발송) {
	setUp();
	EXPECT_CALL(smsSender, send).Times(1);

	Schedule* schedule = new Schedule{ ON_THE_HOUR , UNDER_CAPACITY, CUSTOMER };

	bookingScheduler.addSchedule(schedule);
}

TEST_F(BookingSchedulerTest, 이메일이없는경우에는이메일미발송) {
	setUp();
	EXPECT_CALL(mailSender, sendMail).Times(0);

	Schedule* schedule = new Schedule{ ON_THE_HOUR , UNDER_CAPACITY, CUSTOMER };

	bookingScheduler.addSchedule(schedule);
}

TEST_F(BookingSchedulerTest, 이메일이있는경우에는이메일발송) {
	setUp();
	EXPECT_CALL(mailSender, sendMail).Times(1);

	Schedule* schedule = new Schedule{ ON_THE_HOUR , UNDER_CAPACITY, CUSTOMER_WITH_MAIL };

	bookingScheduler.addSchedule(schedule);
}

TEST_F(BookingSchedulerTest, 현재날짜가일요일인경우예약불가예외처리) {
	setUp();

	tm sundayDate = getTime(2025, 7, 20, 13, 0);
	TestBookingScheduler sunday{ CAPACITY_PER_HOUR, sundayDate };

	try {
		schedule = new Schedule{ ON_THE_HOUR , UNDER_CAPACITY, CUSTOMER };
		sunday.addSchedule(schedule);
		FAIL();
	}
	catch (std::exception& e) {
		EXPECT_EQ(string{ e.what() }, "Booking system is not available on sunday");
	}
}

TEST_F(BookingSchedulerTest, 현재날짜가일요일이아닌경우예약가능) {
	setUp();

	tm mondayDate = getTime(2025, 7, 21, 13, 0);
	TestBookingScheduler monday{ CAPACITY_PER_HOUR, mondayDate };

	Schedule* schedule = new Schedule{ ON_THE_HOUR , CAPACITY_PER_HOUR, CUSTOMER };

	monday.addSchedule(schedule);
	EXPECT_EQ(true, monday.hasSchedule(schedule));
}

int main() {
	::testing::InitGoogleMock();
	return RUN_ALL_TESTS();
}