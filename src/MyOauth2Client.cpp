/*

INSTRUCTIONS

This sample performs authorization code grant flow on various OAuth 2.0
services and then requests basic user information.

This sample is for Windows Desktop, OS X and Linux.
Execute with administrator privileges.

Set the app key & secret strings below (i.e. s_dropbox_key, s_dropbox_secret, etc.)
To get key & secret, register an app in the corresponding service.

Set following entry in the hosts file:
127.0.0.1    testhost.local

*/

#include "cpprest/http_listener.h"
#include "cpprest/http_client.h"

using namespace utility;
using namespace web;
using namespace web::http;
using namespace web::http::client;
using namespace web::http::oauth2::experimental;
using namespace web::http::experimental::listener;

//
// Set key & secret pair to enable session for that service.
//
static const utility::string_t s_dropbox_key(U("199281123173-s69303ojtuegf7n5pf178kil33c53v1i.apps.googleusercontent.com"));
static const utility::string_t s_dropbox_secret(U("oFQK3ELzm64e-D5BdecUL-j_"));

static const utility::string_t s_linkedin_key(U(""));
static const utility::string_t s_linkedin_secret(U(""));

static const utility::string_t s_live_key(U(""));
static const utility::string_t s_live_secret(U(""));

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
    oauth2_code_listener(
        uri listen_uri,
        oauth2_config& config) :
            m_listener(new http_listener(listen_uri)),
            m_config(config)
    {
        m_listener->support([this](http::http_request request) -> void
        {
            if (request.request_uri().path() == U("/") && request.request_uri().query() != U(""))
            {
                m_resplock.lock();

                m_config.token_from_redirected_uri(request.request_uri()).then([this,request](pplx::task<void> token_task) -> void
                {
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

    ~oauth2_code_listener()
    {
        m_listener->close().wait();
    }

    pplx::task<bool> listen_for_code()
    {
        return pplx::create_task(m_tce);
    }

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
    oauth2_session_sample(utility::string_t name,
        utility::string_t client_key,
        utility::string_t client_secret,
        utility::string_t auth_endpoint,
        utility::string_t token_endpoint,
        utility::string_t redirect_uri) :
            m_oauth2_config(client_key,
                client_secret,
                auth_endpoint,
                token_endpoint,
                redirect_uri),
            m_name(name),
            m_listener(new oauth2_code_listener(redirect_uri, m_oauth2_config))
    {}

    void run()
    {
        if (is_enabled())
        {
            std::cout << "Running " << m_name.c_str() << " session..." << "\n";

            if (!m_oauth2_config.token().is_valid_access_token())
            {
                if (authorization_code_flow().get())
                {
                    m_http_config.set_oauth2(m_oauth2_config);
                }
                else
                {
                    std::cout << "Authorization failed for " << m_name.c_str() << "." << "\n";
                }
            }

            run_internal();
        }
        else
        {
            std::cout << "Skipped " << m_name.c_str() << " session sample because app key or secret is empty. Please see instructions." << "\n";
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
    bool is_enabled() const
    {
        return !m_oauth2_config.client_key().empty() && !m_oauth2_config.client_secret().empty();
    }

    void open_browser_auth()
    {
        auto auth_uri(m_oauth2_config.build_authorization_uri(true));
        std::cout << "Opening browser in URI:" << "\n";
        std::cout << auth_uri << "\n";
        open_browser(auth_uri);
    }

    utility::string_t m_name;
    std::unique_ptr<oauth2_code_listener> m_listener;
};

//
// Specialized class for Dropbox OAuth 2.0 session.
//
class dropbox_session_sample : public oauth2_session_sample
{
public:
    dropbox_session_sample() :
        oauth2_session_sample(U("YouTube"),
            s_dropbox_key,
            s_dropbox_secret,
            U("https://accounts.google.com/o/oauth2/auth"),
            U("https://accounts.google.com/o/oauth2/token"),
            U("http://localhost:8889/"))
    {
        // Dropbox uses "default" OAuth 2.0 settings.
    }

protected:
    void run_internal() override
    {
        http_client api(U("https://accounts.google.com/o/oauth2/auth?"), m_http_config);
        std::cout << "Requesting account information:" << "\n";
        std::cout << "Information: " << api.request(methods::GET, U("account/info")).get().extract_json().get() << "\n";
    }
};

//
// Specialized class for LinkedIn OAuth 2.0 session.
//
class linkedin_session_sample : public oauth2_session_sample
{
public:
    linkedin_session_sample() :
        oauth2_session_sample(U("LinkedIn"),
            s_linkedin_key,
            s_linkedin_secret,
            U("https://www.linkedin.com/uas/oauth2/authorization"),
            U("https://www.linkedin.com/uas/oauth2/accessToken"),
            U("http://localhost:8888/"))
    {
        // LinkedIn doesn't use bearer auth.
        m_oauth2_config.set_bearer_auth(false);
        // Also doesn't use HTTP Basic for token endpoint authentication.
        m_oauth2_config.set_http_basic_auth(false);
        // Also doesn't use the common "access_token", but "oauth2_access_token".
        m_oauth2_config.set_access_token_key(U("oauth2_access_token"));
    }

protected:
    void run_internal() override
    {
        http_client api(U("https://api.linkedin.com/v1/people/"), m_http_config);
        std::cout << "Requesting account information:" << "\n";
        std::cout << "Information: " << api.request(methods::GET, U("~?format=json")).get().extract_json().get() << "\n";
    }

};

//
// Specialized class for Microsoft Live Connect OAuth 2.0 session.
//
class live_session_sample : public oauth2_session_sample
{
public:
    live_session_sample() :
        oauth2_session_sample(U("Live"),
            s_live_key,
            s_live_secret,
            U("https://login.live.com/oauth20_authorize.srf"),
            U("https://login.live.com/oauth20_token.srf"),
            U("http://testhost.local:8890/"))
    {
        // Scope "wl.basic" allows fetching user information.
        m_oauth2_config.set_scope(U("wl.basic"));
    }

protected:
    void run_internal() override
    {
        http_client api(U("https://apis.live.net/v5.0/"), m_http_config);
        std::cout << "Requesting account information:" << "\n";
        std::cout << api.request(methods::GET, U("me")).get().extract_json().get() << "\n";
    }
};


int main(int argc, char *argv[]) try
{
    std::cout << "Running OAuth 2.0 client sample..." << "\n";

//    linkedin_session_sample linkedin;
    dropbox_session_sample  dropbox;
//    live_session_sample     live;

//    linkedin.run();
    dropbox.run();
//    live.run();

    std::cout << "Done." << "\n";
    return 0;
}
catch(const std::exception& e)
{
   std::cout << "ERROR: " << e.what() << "\n";
}
