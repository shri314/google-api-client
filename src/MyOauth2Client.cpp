#include <fstream>

#include "cpprest/http_client.h"
#include "cpprest/http_listener.h"

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
   std::unique_ptr<http_listener>    m_listener;
   pplx::task_completion_event<bool> m_tce;
   oauth2_config&                    m_config;
   std::mutex                        m_resplock;
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
   oauth2_config      m_oauth2_config;

private:
   bool is_enabled() const { return !m_oauth2_config.client_key().empty() && !m_oauth2_config.client_secret().empty(); }
   void open_browser_auth()
   {
      auto auth_uri(m_oauth2_config.build_authorization_uri(true));
      std::cout << "Opening browser in URI:\n";
      std::cout << auth_uri << "\n";
      open_browser(auth_uri);
   }

   utility::string_t                     m_name;
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

int main(int argc, char* argv[]) try
{
   std::cout << "Running OAuth 2.0 client sample...\n";

   //   youtube_session_sample youtube;
   //   youtube.run();

   std::cout << "Done.\n";

   utility::string_t listen_uri     = U("http://localhost:8888/");
   utility::string_t name           = U("YouTube");
   utility::string_t client_key     = s_youtube_key;
   utility::string_t client_secret  = s_youtube_secret;
   utility::string_t auth_endpoint  = U("https://accounts.google.com/o/oauth2/auth");
   utility::string_t token_endpoint = U("https://accounts.google.com/o/oauth2/token");
   utility::string_t redirect_uri   = listen_uri + "save_code";
   utility::string_t scope          = U("https://www.googleapis.com/auth/youtube");

   http_listener L(listen_uri);
   L.support(
       [=](http::http_request request) -> void {
          if (request.request_uri().path() == "/home")
          {
             oauth2_config cfg{client_key, client_secret, auth_endpoint, token_endpoint, redirect_uri, scope};

             auto x = cfg.build_authorization_uri(true);

             request.reply(status_codes::OK, U(R"(
                           <html>
                           <body>
                              Welcome! <br>
                              <a href=")" + x + R"(">get_code</a>
                           </body>
                           </html>)"),
                           U("text/html; charset=utf-8"));
          }
          if (request.request_uri().path() == "/load_code")
          {
             utility::string_t  code;
             std::ifstream      file("code.txt", std::ios::binary);
             std::ostringstream oss;
             oss << file.rdbuf();
             oss.str();
             request.reply(status_codes::OK, U("old code = " + oss.str() + "\n"));
          }
          if (request.request_uri().path() == "/save_code")
          {
             if (request.request_uri().query() != U(""))
             {
                auto&& code = request.request_uri().query();

                {
                   std::ofstream file("code.txt", std::ios::binary);
                   file << code;
                }
                request.reply(status_codes::OK, U("code saved"));
             }
          }
          else
          {
             std::cout << "not found.\n";
             request.reply(status_codes::NotFound, U("not found."));
          }
       });

   auto&& f = L.open();

   f.wait();

   open_browser(listen_uri + "home");

   return 0;
}
catch (const std::exception& e)
{
   std::cout << "ERROR: " << e.what() << "\n";
}
