#include "SessionManager.hpp"
#include <fstream>

std::map<std::string, int> SessionManager::_sessions;

std::string SessionManager::generateId()
{
    static bool avoid_reuse = false;
    if (!avoid_reuse) {
        std::srand(time(NULL));
        avoid_reuse = true;
    }
    std::stringstream ss;
    ss << "sess_" << std::rand();
    return ss.str();
}

std::string SessionManager::get_cookies_val(const std::string &cookies_header, const std::string &key)
{
    size_t pos = cookies_header.find(key);
    if (pos == std::string::npos) return "";
    pos += key.length();
    size_t end = cookies_header.find(";", pos);
    if (end == std::string::npos) end = cookies_header.length();
    return cookies_header.substr(pos, end - pos);
}

// Simple C++98 string replacement helper function
static std::string replaceMarker(std::string str, const std::string& marker, const std::string& replacement) {
    size_t pos = 0;
    while ((pos = str.find(marker, pos)) != std::string::npos) {
        str.replace(pos, marker.length(), replacement);
        pos += replacement.length();
    }
    return str;
}

HttpResponse SessionManager::handle(const HttpRequest &req)
{
    HttpResponse res;
    res.statusCode = 200;
    res.headers["Content-Type"] = "text/html";

    std::string session_id;
    bool isNewSession = false;

    std::map<std::string, std::string>::const_iterator it = req.headers.find("Cookie");
    if (it != req.headers.end())
        session_id = get_cookies_val(it->second, "session_id=");

    if (session_id.empty() || _sessions.find(session_id) == _sessions.end()) {
        isNewSession = true;
        session_id = generateId();
        _sessions[session_id] = 1;
        res.headers["Set-Cookie"] = "session_id=" + session_id + "; Path=/; HttpOnly";
    } else {
        _sessions[session_id]++;
    }

    std::string userAgent = "Unknown Client Engine";
    std::map<std::string, std::string>::const_iterator uaIt = req.headers.find("User-Agent");
    if (uaIt != req.headers.end()) userAgent = uaIt->second;

    // Read the static template HTML file into a buffer stream
    std::ifstream htmlFile("./www/session.html");
    if (!htmlFile.is_open()) {
        res.statusCode = 500;
        res.body = "<h1>500 Internal Server Error</h1><p>Missing session template asset file.</p>";
        return res;
    }
    std::stringstream buffer;
    buffer << htmlFile.rdbuf();
    htmlFile.close();

    std::string pageContent = buffer.str();

    // Construct the dynamic context badge block
    std::string statusHTML;
    if (isNewSession) {
        statusHTML = "<div class='main-card'>"
                     "  <span class='hero-badge badge-new'>"
                     "    <svg width='12' height='12' viewBox='0 0 24 24' fill='none' stroke='currentColor' stroke-width='2' style='margin-right:4px;'><circle cx='12' cy='12' r='10'/><line x1='12' y1='8' x2='12' y2='16'/><line x1='8' y1='12' x2='16' y2='12'/></svg>" // 🌟 Fixed closing slash here
                     "    State Token Allocated"
                     "  </span>"
                     "  <h2 class='status-title'>🎉 Fresh Handshake Executed</h2>"
                     "  <p class='status-desc'>A state-less tracking sequence has been mapped to this environment footprint.</p>"
                     "</div>";
    } else {
        statusHTML = "<div class='main-card'>"
                     "  <span class='hero-badge badge-return'>"
                     "    <svg width='12' height='12' viewBox='0 0 24 24' fill='none' stroke='currentColor' stroke-width='2' style='margin-right:4px;'><path d='M21.5 2v6h-6M21.34 15.57a10 10 0 1 1-.57-8.38l5.67-5.67'/></svg>"
                     "    Restored Active Pipeline"
                     "  </span>"
                     "  <h2 class='status-title'>👋 Welcome back, user</h2>"
                     "  <p class='status-desc'>Your client storage successfully presented a verified tracking hash context.</p>"
                     "</div>";
    }

    // Dynamic replacement pass execution updates
    pageContent = replaceMarker(pageContent, "{{SESSION_STATUS}}", statusHTML);
    pageContent = replaceMarker(pageContent, "{{SESSION_ID}}", session_id);
    
    std::stringstream ssVisits;
    ssVisits << _sessions[session_id];
    pageContent = replaceMarker(pageContent, "{{VISITS}}", ssVisits.str());
    pageContent = replaceMarker(pageContent, "{{USER_AGENT}}", userAgent);

    res.body = pageContent;

    // Add Content-Length
    std::stringstream ssLen;
    ssLen << res.body.length();
    res.headers["Content-Length"] = ssLen.str();

    return res;
}