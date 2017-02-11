#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE header_test
#include <boost/test/unit_test.hpp>

#include <string>

using namespace std::string_literals;

#define STRINGIZE_(x) #x
#define STRINGIZE(x) STRINGIZE_(x)
#define TEST_CONTEXT(x)       __FILE__ + "("s + STRINGIZE(__LINE__) + "): "s + (x)

BOOST_AUTO_TEST_CASE(one_test)
{
   struct
   {
      std::string test_context;
   }
   TestData[] =
   {
      {
         TEST_CONTEXT("context one"),
      },
   };

   /////////////////////////////////////////////////////

   for(auto Datum : TestData)
   {
      BOOST_TEST_CONTEXT(Datum.test_context)
      {
         auto pH = std::make_unique<int>(1); // TEST OBJECT

         /// Set
         {
            *pH = 10;
         }

         /// Check
         {
            BOOST_CHECK_EQUAL(*pH, 10);
         }
      }
   }
}
