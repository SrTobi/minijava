#pragma once
#ifdef TESTX_TEST
#ifndef TESTX_TESTX_HPP
#define TESTX_TESTX_HPP



#include <boost/test/unit_test.hpp>
#include <boost/utility.hpp>
#include <boost/lockfree/queue.hpp>
#include <list>
#include <memory>
#include <mutex>

#define TESTX_PARAM_TEST_CASE(_name, ...)	\
			void _name (__VA_ARGS__)

#define TESTX_AUTO_TEST_CASE(_name) \
			BOOST_AUTO_TEST_CASE(_name)

#define TESTX_PARAM_TEST(_func, ...)	\
			TESTX_PARAM_TEST_NAMED(_func, BOOST_JOIN(_func, BOOST_JOIN(_in_, __LINE__)), __VA_ARGS__)
#define TESTX_PARAM_TEST_NAMED(_func, _name, ...)	\
			BOOST_AUTO_TEST_CASE(_name)	{ _func(__VA_ARGS__); }

#define TESTX_START_FIXTURE_TEST(_test, ...)	\
			TESTX_START_FIXTURE_TEST_NAMED(BOOST_JOIN(_test, BOOST_JOIN(_in_, __LINE__)), _test, __VA_ARGS__)
#define TESTX_START_FIXTURE_TEST_NAMED(_name, _fixture, ...)								\
			struct BOOST_JOIN(_name, _help_fixture)										\
			{																			\
				typedef _fixture fixture;												\
				BOOST_JOIN(_name, _help_fixture)()										\
				{																		\
					m_fix = new fixture(__VA_ARGS__);									\
				}																		\
																						\
				~BOOST_JOIN(_name, _help_fixture)()										\
				{																		\
					delete m_fix;														\
				}																		\
																						\
				fixture* m_fix;															\
			};																			\
			BOOST_FIXTURE_TEST_SUITE(_name, BOOST_JOIN(_name, _help_fixture))			

#define TESTX_END_FIXTURE_TEST()		\
			BOOST_AUTO_TEST_SUITE_END()

#define TESTX_FIXTURE_TEST(_func, ...)	\
			TESTX_FIXTURE_TEST_NAMED(_func, BOOST_JOIN(_func, BOOST_JOIN(_in_, __LINE__)), __VA_ARGS__);
#define TESTX_FIXTURE_TEST_NAMED(_func, _name, ...)	\
			BOOST_AUTO_TEST_CASE(_name) { m_fix->_func(__VA_ARGS__); }

			
namespace testx {

	template<typename Events>
	class MockObserver
	{
	public:
		struct Inserter
		{
			Inserter(std::shared_ptr<boost::lockfree::queue<Events> > events)
				: events(events)
			{
			}

			Inserter& operator << (const Events& v)
			{
				events->push(v);
				return *this;
			}

		private:
			std::shared_ptr<boost::lockfree::queue<Events> > events;
		};

		MockObserver()
		{
			events = std::make_shared<boost::lockfree::queue<Events> >(0);
		}

		~MockObserver()
		{
			if(events.unique())
			{
				BOOST_CHECK(events->empty());
			}
		}

		MockObserver(const MockObserver& other)
		{
			events = other.events;
		}

		void expect(const Events& v)
		{
			Events q;
			BOOST_REQUIRE(events->pop(q));
			BOOST_CHECK_EQUAL(v, q);
		}

		Inserter set()
		{
			return Inserter(events);
		}
		

	private:
		std::shared_ptr<boost::lockfree::queue<Events> > events;
	};

}

#endif
#endif