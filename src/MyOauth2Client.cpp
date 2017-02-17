#include <fstream>

#include "cpprest/http_client.h"
#include "cpprest/http_listener.h"
#include <boost/lexical_cast.hpp>

using namespace utility;
using namespace web;
using namespace web::http;
using namespace web::http::client;
using namespace web::http::oauth2::experimental;
using namespace web::http::experimental::listener;

//
// Set key & secret pair to enable session for that service.
//
static const utility::string_t s_youtube_key(U("199281123173-s69303ojtuegf7n5pf178kil33c53v1i.apps.googleusercontent.com"));
static const utility::string_t s_youtube_secret(U("oFQK3ELzm64e-D5BdecUL-j_"));

//
// Utility method to open browser on Windows, OS X and Linux systems.
//
static void open_browser(utility::string_t auth_uri)
{
   string_t browser_cmd(U("xdg-open \"") + auth_uri + U("\""));
   system(browser_cmd.c_str());
}

//
// A simple listener class to capture OAuth 2.0 HTTP redirect to localhost.
// The listener captures redirected URI and obtains the token.
// This type of listener can be implemented in the back-end to capture and store tokens.
//
class oauth2_code_listener
{
public:
   oauth2_code_listener(uri listen_uri, oauth2_config& config)
      : m_listener(new http_listener(listen_uri))
      , m_config(config)
   {
      m_listener->support([this](http::http_request request) -> void {
         if (request.request_uri().path() == U("/") && request.request_uri().query() != U(""))
         {
            m_resplock.lock();

            // std::cout << "Got URI: " << request.request_uri() << "\n";

            m_config.token_from_redirected_uri(request.request_uri()).then([this, request](pplx::task<void> token_task) -> void {
               try
               {
                  token_task.wait();
                  m_tce.set(true);
               }
               catch (const oauth2_exception& e)
               {
                  std::cout << "Error: " << e.what() << "\n";
                  m_tce.set(false);
               }
            });

            request.reply(status_codes::OK, U("Ok."));

            m_resplock.unlock();
         }
         else
         {
            request.reply(status_codes::NotFound, U("Not found."));
         }
      });

      m_listener->open().wait();
   }

   ~oauth2_code_listener() { m_listener->close().wait(); }
   pplx::task<bool> listen_for_code() { return pplx::create_task(m_tce); }
private:
   std::unique_ptr<http_listener> m_listener;
   pplx::task_completion_event<bool> m_tce;
   oauth2_config& m_config;
   std::mutex m_resplock;
};

//
// Base class for OAuth 2.0 sessions of this sample.
//
class oauth2_session_sample
{
public:
   oauth2_session_sample(utility::string_t name, utility::string_t client_key, utility::string_t client_secret, utility::string_t auth_endpoint, utility::string_t token_endpoint,
                         utility::string_t redirect_uri, utility::string_t scope = "")
      : m_oauth2_config(client_key, client_secret, auth_endpoint, token_endpoint, redirect_uri, scope)
      , m_name(name)
      , m_listener(std::make_unique<oauth2_code_listener>(redirect_uri, m_oauth2_config))
   {
   }

   void run()
   {
      if (is_enabled())
      {
         std::cout << "Running " << m_name.c_str() << " session...\n";

         if (!m_oauth2_config.token().is_valid_access_token())
         {
            if (authorization_code_flow().get())
            {
               m_http_config.set_oauth2(m_oauth2_config);
            }
            else
            {
               std::cout << "Authorization failed for " << m_name.c_str() << ".\n";
            }
         }

         run_internal();
      }
      else
      {
         std::cout << "Skipped " << m_name.c_str() << " session sample because app key or secret is empty. Please see instructions.\n";
      }
   }

protected:
   virtual void run_internal() = 0;

   pplx::task<bool> authorization_code_flow()
   {
      open_browser_auth();
      return m_listener->listen_for_code();
   }

   http_client_config m_http_config;
   oauth2_config m_oauth2_config;

private:
   bool is_enabled() const { return !m_oauth2_config.client_key().empty() && !m_oauth2_config.client_secret().empty(); }
   void open_browser_auth()
   {
      auto auth_uri(m_oauth2_config.build_authorization_uri(true));
      std::cout << "Opening browser in URI:\n";
      std::cout << auth_uri << "\n";
      open_browser(auth_uri);
   }

   utility::string_t m_name;
   std::unique_ptr<oauth2_code_listener> m_listener;
};

//
// Specialized class for YouTube OAuth 2.0 session.
//
class youtube_session_sample : public oauth2_session_sample
{
public:
   youtube_session_sample()
      : oauth2_session_sample(U("YouTube"), s_youtube_key, s_youtube_secret, U("https://accounts.google.com/o/oauth2/auth"), U("https://accounts.google.com/o/oauth2/token"),
                              U("http://localhost:8889/"), U("https://www.googleapis.com/auth/youtube"))
   {
      // youtube uses "default" OAuth 2.0 settings.
   }

protected:
   void run_internal() override
   {
      http_client api(U("https://www.googleapis.com/youtube/v3/playlistItems?part=id&playlistId=HL"), m_http_config);

      std::cout << "Requesting account information:\n";

      std::cout << "Information: " << api.request(methods::GET, U("account/info")).get().extract_json().get() << "\n";
   }
};


struct my_oauth2_params
{
   utility::string_t client_key;
   utility::string_t client_secret;
   utility::string_t auth_endpoint;
   utility::string_t token_endpoint;
   utility::string_t scope;
};


auto&& save_token = [](std::ostream& os, const oauth2_token& tok) {
   os << tok.access_token() << "\n";
   os << tok.refresh_token() << "\n";
   os << tok.token_type() << "\n";
   os << tok.scope() << "\n";
   os << tok.expires_in() << "\n";
};


auto&& print_token = [](std::ostream& os, const oauth2_token& tok) {
   os << "valid = " << tok.is_valid_access_token() << "\n";
   os << "access_token = " << tok.access_token() << "\n";
   os << "refresh_token = " << tok.refresh_token() << "\n";
   os << "token_type = " << tok.token_type() << "\n";
   os << "scope = " << tok.scope() << "\n";
   os << "expires_in = " << tok.expires_in() << "\n";
};

auto&& load_token = [](std::istream& is) {
   auto&& read_str = [&is]() {
      utility::string_t str;
      std::getline(is, str);
      return str;
   };
   auto&& read_int64_t = [&is]() {
      utility::string_t str;
      std::getline(is, str);
      int64_t x;
      std::istringstream iss(str);
      iss >> x;
      return x;
   };

   oauth2_token tok;
   tok.set_access_token(read_str());
   tok.set_refresh_token(read_str());
   tok.set_token_type(read_str());
   tok.set_scope(read_str());
   tok.set_expires_in(read_int64_t());

   return tok;
};

int main(int argc, char* argv[]) try
{
   std::mutex m;
   std::condition_variable cv;
   bool quit = false;

   utility::string_t listen_uri = U("http://localhost:8888/");

   my_oauth2_params params{s_youtube_key, s_youtube_secret, U("https://accounts.google.com/o/oauth2/auth"), U("https://accounts.google.com/o/oauth2/token"),
                           U("https://www.googleapis.com/auth/youtube")};

   oauth2_config cfg{params.client_key, params.client_secret, params.auth_endpoint, params.token_endpoint, "", params.scope};

   http_listener L(listen_uri);

   L.support(
      [&](http::http_request request) -> void {
         if (request.request_uri().path() == "/")
         {
            cfg.set_redirect_uri(listen_uri + "save_token");

            auto x = cfg.build_authorization_uri(true);

            request.reply(status_codes::OK, U(R"(
                          <html>
                          <body>
                             Welcome to YouTube Proxy! <br>
                             <a href=")" + x + R"(">get_code</a>
                          </body>
                          </html>)"),
                          U("text/html; charset=utf-8"));
         }
         else if (request.request_uri().path() == "/load_token")
         {
            std::ifstream file("store.txt", std::ios::binary);
            oauth2_token tk = load_token(file);

            std::ostringstream oss;
            print_token(oss, tk);

            request.reply(status_codes::OK, U("loaded token - " + oss.str()));
         }
         else if (request.request_uri().path() == "/quit")
         {
            std::unique_lock<std::mutex> lk(m);
            quit = true;
            cv.notify_all();

            request.reply(status_codes::OK, U("stopping."));
         }
         else if (request.request_uri().path() == "/save_token")
         {
            auto&& f = cfg.token_from_redirected_uri(request.request_uri());
            f.wait();

            auto&& tk = cfg.token();

            std::ofstream file("store.txt", std::ios::binary);
            save_token(file, tk);

            std::ostringstream oss;
            print_token(oss, tk);

            request.reply(status_codes::OK, U("saved token\n" + oss.str()));
         }
         else
         {
            std::cout << "not found.\n";
            request.reply(status_codes::NotFound, U("not found."));
         }
      });

   std::cout << "running HTTP server at uri - " << listen_uri << "\n";

   L.open().wait();

   std::unique_lock<std::mutex> lk(m);
   cv.wait(lk, [&quit] { return quit; });

   return 0;
}
catch (const std::exception& e)
{
   std::cout << "ERROR: " << e.what() << "\n";
}
