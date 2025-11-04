


MDTP (Markdown Transfer Protocol) 
=================================== 


------------------------------------ 
Version: 1.0
Author: Trevohack 
------------------------------------ 


= *OVERVIEW* = 

MDTP is a lightweight, text-based protocol designed for serving and transmitting Markdown documents directly between clients and servers. It functions similarly to HTTP but is simplified, focusing on plain Markdown content instead of HTML. MDTP allows fast and minimalistic document delivery without requiring traditional web technologies like HTML, CSS, or JavaScript.

The MDTP ecosystem includes:

* MDTP Server
* MDTP Client
* Protocol Parser
* Request/Response Handler
* MDTP Bridge (for viewing MDTP pages in standard web browsers)

---

1. Core Concept

---

MDTP operates on its own protocol layer using TCP sockets (default port: 8585).
It uses a simple, human-readable request and response structure that mirrors HTTP,
but is explicitly optimized for Markdown transmission.

Example request:
------------------------------------------------------------------------------------
| GET /index.md MDTP/1.0                                                             |
| Host: 127.0.0.1                                                                    | 
| User-Agent: MDTP-Client/1.0                                                        |
| Accept: text/markdown                                                              |
------------------------------------------------------------------------------------ 


Example response: 
-------------------------------------------------------------------------------------- 
| MDTP/1.0 200 OK                                                                    |
| Content-Type: text/markdown                                                        |
| Content-Length: 1243                                                               |
| Date: Sun, 03 Nov 2025 18:00:00 GMT                                                |
| Server: MDTP-Server/1.0                                                            |
|                                                                                    | 
| # Welcome to MDTP                                                                  |
|                                                                                    |
| This document was served using the Markdown Transfer Protocol.                     |
-------------------------------------------------------------------------------------- 



2. MDTP Server


The MDTP server listens for incoming connections on a specified port (default 8585).
It reads client requests, parses the headers, and returns Markdown files from the
current working directory.

The request parser extracts the method (GET), path, version, host, and user agent.
Supported methods:

* GET  (Retrieve Markdown documents)

The server locates files based on the requested path.
If the request is "/", it automatically serves "./index.md".
Files are read in text mode and served with Content-Type: text/markdown.

If a requested file does not exist, the server responds with a Markdown 404 page.

---

3. Status Codes

---

MDTP defines several standard status codes for responses:

200  OK                 - Request successful, document returned.
400  Bad Request        - Malformed request or invalid syntax.
404  Not Found          - Requested document not found on server.
500  Internal Error     - Server encountered an unexpected condition.

Each response includes a timestamp, server signature, and content length.
The server dynamically generates headers using get_timestamp() for accurate time
stamping in GMT format.

---

4. MDTP Client

---

The built-in MDTP client connects to an MDTP server via TCP, sends a request, and
receives Markdown-formatted data.

It builds a request similar to:
GET /index.md MDTP/1.0
Host: 127.0.0.1
User-Agent: MDTP-Client/1.0

The client then extracts the body after the double line break ("\r\n\r\n")
and prints the raw Markdown content directly to the console.

Example usage:
./mdtp client 127.0.0.1 /index.md

---

5. MDTP Bridge

---

Since web browsers like Firefox and Chrome cannot natively handle the "mdtp://"
protocol, the MDTP Bridge was created to allow seamless viewing of MDTP sites
within any standard browser.

The Bridge runs as a standalone server on port 9999 (default).
It acts as an HTTP-to-MDTP translator:

* Listens for HTTP requests from browsers ([http://127.0.0.1:9999/](http://127.0.0.1:9999/))
* Extracts the target MDTP host and path (for example: /127.0.0.1:8585/index.md)
* Connects to the MDTP server using the internal fetch_mdtp() function
* Converts the received Markdown into styled HTML using markdown_to_html()
* Sends the final HTML response back to the browser

The Bridge applies a clean, modern, CSS-styled template for better readability.
It supports headings, lists, code blocks, links, bold, italics, and inline code.

When running, it displays:

╔════════════════════════════════════════════════════════╗
║       MDTP HTTP Bridge Server - Running!               ║
╠════════════════════════════════════════════════════════╣
║  Browse MDTP sites in your regular web browser!        ║
║  Open: [http://127.0.0.1:9999/](http://127.0.0.1:9999/)║                        
║  Direct: [http://127.0.0.1:9999/127.0.0.1:8585/index.md║
╚════════════════════════════════════════════════════════╝

---

6. How It Works Internally

---

Step 1:  The MDTP client or bridge sends a raw MDTP request using TCP.
Step 2:  The MDTP server receives and parses the request.
Step 3:  It locates the corresponding Markdown file in the current directory.
Step 4:  The server constructs a valid MDTP response header and sends it.
Step 5:  The client extracts the Markdown body from the response.
Step 6:  The bridge (if used) converts the Markdown into styled HTML for browsers.

This flow ensures a completely text-based transfer layer for human-readable content,
removing all unnecessary overhead from traditional web stacks.

---

7. Design Highlights

---

• Lightweight Markdown-first protocol
• Fully readable and debuggable text-based exchange
• Minimal dependencies, pure C socket-based networking
• Built-in status codes and server response handling
• Bridge layer for browser compatibility
• Clear separation between protocol and rendering layer

---

8. Usage Summary

---

Start the MDTP server:
./mdtp server 8585

Access it via the built-in client:
./mdtp client 127.0.0.1 /index.md

Or access it through your web browser:
[http://127.0.0.1:9999/127.0.0.1:8585/index.md](http://127.0.0.1:9999/127.0.0.1:8585/index.md)

---

9. Future Extensions

---

Planned features include:

* MDTP Secure (MDTPS) using TLS
* Rich metadata headers (Author, Theme, Version)
* Extended Markdown directives for custom UI
* Native MDTP browser with animated rendering
* System-level mdtp:// URI registration

---

10. License

---

MDTP 1.0 is released for educational and experimental purposes.
You are free to modify, extend, or integrate the protocol into your own systems
for non-commercial or research projects.

---

EOF 
