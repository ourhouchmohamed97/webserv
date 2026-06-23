#pragma once

#include <string>
#include <sstream>

class ErrorPageFactory {
private:
    static std::string buildTemplate(const std::string& code, const std::string& title, const std::string& description) {
        std::string html;
        html += "<!DOCTYPE html>\n<html lang=\"en\">\n<head>\n";
        html += "  <meta charset=\"UTF-8\">\n";
        html += "  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n";
        html += "  <title>Webserv Dashboard — Error " + code + "</title>\n";
        
        html += "  <style>\n";
        html += "    @import url('https://fonts.googleapis.com/css2?family=Inter:wght@400;500;600;700;800&family=Playfair+Display:wght@700&display=swap');\n";
        html += "    :root {\n";
        html += "      --bg-primary: #0a0a0f;\n";
        html += "      --bg-navbar: rgba(10, 10, 18, 0.85);\n";
        html += "      --bg-card: #0f0f18;\n";
        html += "      --border-default: #1e1e2e;\n";
        html += "      --border-subtle: #16161f;\n";
        html += "      --text-primary: #ffffff;\n";
        html += "      --text-secondary: #9999aa;\n";
        html += "      --text-muted: #666677;\n";
        html += "      --accent-purple: #7c3aed;\n";
        html += "      --accent-purple-dark: #6d28d9;\n";
        html += "      --accent-purple-light: #a78bfa;\n";
        html += "      --accent-red: #ef4444;\n";
        html += "      --accent-red-bg: rgba(239, 68, 68, 0.1);\n";
        html += "    }\n";
        html += "    *, *::before, *::after { box-sizing: border-box; margin: 0; padding: 0; }\n";
        html += "    body { font-family: 'Inter', sans-serif; background-color: var(--bg-primary); color: var(--text-secondary); line-height: 1.6; min-height: 100vh; display: flex; flex-direction: column; }\n";
        html += "    .navbar { position: fixed; top: 0; left: 0; right: 0; height: 60px; background: var(--bg-navbar); backdrop-filter: blur(20px); -webkit-backdrop-filter: blur(20px); border-bottom: 1px solid var(--border-subtle); display: flex; align-items: center; z-index: 1000; }\n";
        html += "    .container { max-width: 1200px; margin: 0 auto; padding: 0 24px; width: 100%; display: flex; align-items: center; justify-content: space-between; }\n";
        html += "    .nav-links { display: flex; align-items: center; gap: 4px; }\n";
        html += "    .nav-link { display: flex; align-items: center; gap: 6px; padding: 8px 14px; border-radius: 8px; font-size: 14px; font-weight: 500; color: var(--text-secondary); text-decoration: none; }\n";
        html += "    .nav-icon { width: 16px; height: 16px; opacity: 0.7; fill: none; stroke: currentColor; stroke-width: 2; }\n";
        html += "    .page-content { padding-top: 60px; flex: 1; display: flex; align-items: center; justify-content: center; }\n";
        html += "    .error-card { background: var(--bg-card); border: 1px solid var(--border-default); border-radius: 12px; padding: 48px; max-width: 500px; width: 100%; text-align: center; box-shadow: 0 4px 24px rgba(0,0,0,0.3); }\n";
        html += "    .hero-badge { display: inline-flex; align-items: center; gap: 6px; padding: 4px 12px; border-radius: 6px; font-size: 11px; font-weight: 600; text-transform: uppercase; letter-spacing: 1px; background: var(--accent-red-bg); color: var(--accent-red); border: 1px solid rgba(239, 68, 68, 0.25); margin-bottom: 24px; }\n";
        html += "    .error-code { font-family: 'Playfair Display', Georgia, serif; font-size: 6rem; font-weight: 900; line-height: 1; color: var(--text-primary); margin-bottom: 8px; letter-spacing: -2px; }\n";
        html += "    .error-title { font-size: 1.5rem; color: var(--text-primary); margin-bottom: 16px; font-weight: 700; }\n";
        html += "    .error-desc { color: var(--text-muted); font-size: 14px; margin-bottom: 32px; }\n";
        html += "    .btn { display: inline-flex; align-items: center; gap: 8px; padding: 12px 24px; border-radius: 8px; font-size: 14px; font-weight: 600; text-decoration: none; transition: all 0.25s ease; border: 1px solid transparent; }\n";
        html += "    .btn-primary { background: var(--accent-purple); color: var(--text-primary); border-color: var(--accent-purple); }\n";
        html += "    .btn-primary:hover { background: var(--accent-purple-dark); box-shadow: 0 0 20px rgba(124, 58, 237, 0.15); transform: translateY(-1px); }\n";
        html += "  </style>\n</head>\n<body>\n";

        html += "  <nav class=\"navbar\">\n    <div class=\"container\">\n      <div class=\"nav-links\">\n";
        html += "        <a href=\"/\" class=\"nav-link\"><svg class=\"nav-icon\" viewBox=\"0 0 24 24\"><path d=\"M3 9l9-7 9 7v11a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2z\"/><polyline points=\"9 22 9 12 15 12 15 22\"/></svg>Home</a>\n";
        html += "        <a href=\"/upload.html\" class=\"nav-link\"><svg class=\"nav-icon\" viewBox=\"0 0 24 24\"><path d=\"M21 15v4a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2v-4\"/><polyline points=\"17 8 12 3 7 8\"/><line x1=\"12\" y1=\"3\" x2=\"12\" y2=\"15\"/></svg>Upload</a>\n";
        html += "      </div>\n    </div>\n  </nav>\n";

        html += "  <main class=\"page-content\">\n";
        html += "    <div class=\"error-card\">\n";
        html += "      <span class=\"hero-badge\">\n";
        html += "        <svg width=\"12\" height=\"12\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\"><circle cx=\"12\" cy=\"12\" r=\"10\"/><line x1=\"15\" y1=\"9\" x2=\"9\" y2=\"15\"/><line x1=\"9\" y1=\"9\" x2=\"15\" y2=\"15\"/></svg>\n";
        html += "        Core Protocol Intercept\n";
        html += "      </span>\n";
        html += "      <div class=\"error-code\">" + code + "</div>\n";
        html += "      <h2 class=\"error-title\">" + title + "</h2>\n";
        html += "      <p class=\"error-desc\">" + description + "</p>\n";
        html += "      <a href=\"/\" class=\"btn btn-primary\">\n";
        html += "        Return to Dashboard\n";
        html += "        <svg width=\"16\" height=\"16\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\"><line x1=\"5\" y1=\"12\" x2=\"19\" y2=\"12\"/><polyline points=\"12 5 19 12 12 19\"/></svg>\n";
        html += "      </a>\n";
        html += "    </div>\n  </main>\n</body>\n</html>";

        return html;
    }

public:
    static std::string getErrorPage(int statusCode) {
        std::stringstream ss;
        ss << statusCode;
        std::string codeStr = ss.str();

        if (statusCode == 400) return buildTemplate(codeStr, "400 Bad Request", "The server could not process the execution parameters or structural formatting token arrays.");
        if (statusCode == 403) return buildTemplate(codeStr, "403 Forbidden", "Access verification denied. Your authorization group lacks permissions for this route path endpoint.");
        if (statusCode == 404) return buildTemplate(codeStr, "404 Not Found", "The requested data block or infrastructure asset path could not be resolved on this system volume.");
        if (statusCode == 405) return buildTemplate(codeStr, "405 Method Not Allowed", "The requested transaction interaction primitive is strictly barred for this specific route location block.");
        if (statusCode == 411) return buildTemplate(codeStr, "411 Length Required", "A valid Content-Length verification metric was completely missing from the transaction header map array.");
        if (statusCode == 413) return buildTemplate(codeStr, "413 Payload Too Large", "The incoming file stream or data body allocation limits exceed our configured server core protection profiles.");
        if (statusCode == 500) return buildTemplate(codeStr, "500 Internal Error", "An unhandled execution trace fault disrupted the backend context runtime process pool.");
        
        return buildTemplate(codeStr, "Protocol Error", "An unmapped connection tracking or transaction state anomaly was intercepted.");
    }
};