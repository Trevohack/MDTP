╔════════════════════════════════════════════════════════════════════════════════════════════════════════════════╗
║                                       MDTP (Markdown Transfer Protocol)                                        ║
╚════════════════════════════════════════════════════════════════════════════════════════════════════════════════╝

──────────────────────────────────────────────────────────────────────────────────────────────────────────────────
 Version : 1.0
 Author  : Trevohack
──────────────────────────────────────────────────────────────────────────────────────────────────────────────────



===========================================================================================
                                * O V E R V I E W *
===========================================================================================

MDTP is a lightweight, text-based protocol designed for serving and transmitting Markdown 
documents directly between clients and servers. It functions similarly to HTTP but is 
simplified and optimized for plain Markdown — no HTML, no CSS, no JavaScript.  

The MDTP ecosystem includes:
   • MDTP Server
   • MDTP Client
   • Protocol Parser
   • Request/Response Handler
   • MDTP Bridge (browser-compatible translator)


===========================================================================================
                                1.  C O R E   C O N C E P T
===========================================================================================

MDTP operates as its own transport-layer protocol over TCP (default port: 8585).  
It follows a request/response model similar to HTTP, but uses Markdown as the payload.  

Example request:
┌────────────────────────────────────────────────────────────────────────────────────────────┐
│ GET /index.md MDTP/1.0                                                                     │
│ Host: 127.0.0.1                                                                            │
│ User-Agent: MDTP-Client/1.0                                                                │
│ Accept: text/markdown                                                                      │
└────────────────────────────────────────────────────────────────────────────────────────────┘

Example response:
┌────────────────────────────────────────────────────────────────────────────────────────────┐
│ MDTP/1.0 200 OK                                                                            │
│ Content-Type: text/markdown                                                                │
│ Content-Length: 1243                                                                       │
│ Date: Sun, 03 Nov 2025 18:00:00 GMT                                                        │
│ Server: MDTP-Server/1.0                                                                    │
│                                                                                            │
│ # Welcome to MDTP                                                                          │
│                                                                                            │
│ This document was served using the Markdown Transfer Protocol.                             │
└────────────────────────────────────────────────────────────────────────────────────────────┘


===========================================================================================
                                2.  S E R V E R   M O D U L E
===========================================================================================

The MDTP Server listens for incoming TCP connections and processes Markdown requests.  
It parses the method, path, version, host, and user-agent, returning Markdown files from 
the local directory.

Supported method: GET  
If “/” is requested, “./index.md” is served by default.  

If a file does not exist, the server generates a 404 Markdown error document.


===========================================================================================
                                3.  S T A T U S   C O D E S
===========================================================================================

 ┌──────┬─────────────────────────────┐
 │ Code │ Description                 │
 ├──────┼─────────────────────────────┤
 │ 200  │ OK – Request successful.    │
 │ 400  │ Bad Request – Invalid data. │
 │ 404  │ Not Found – File missing.   │
 │ 500  │ Internal Error – Exception. │
 └──────┴─────────────────────────────┘

Every MDTP response includes:
   • Timestamp (GMT)
   • Server signature
   • Content-Type: text/markdown
   • Content-Length header


===========================================================================================
                                4.  C L I E N T   M O D U L E
===========================================================================================

The MDTP Client connects to the server, sends a GET request, and prints the Markdown body.  

Example usage:
   ./mdtp client 127.0.0.1 /index.md

Workflow:
   [ CLIENT ]  →  builds request  →  [ SERVER ]
   [ SERVER ]  →  reads Markdown  →  [ CLIENT ]
   [ CLIENT ]  →  renders Markdown to console


===========================================================================================
                                5.  M D T P   B R I D G E
===========================================================================================

Browsers can’t natively handle “mdtp://” links — so the **MDTP Bridge** exists.  
It runs an HTTP server (port 9999) that translates MDTP pages to HTML on the fly.

Bridge Flow:
   ┌──────────────────────────────────────────────┐
   │ Browser (HTTP)  →  MDTP Bridge  →  MDTP Server│
   │ http://127.0.0.1:9999/127.0.0.1:8585/index.md │
   └──────────────────────────────────────────────┘

The bridge fetches Markdown, converts it to HTML using an internal parser,  
then applies a modern styled template (supports bold, italic, code blocks, lists, etc).

Example banner:
╔══════════════════════════════════════════════════════════════════════╗
║                  MDTP HTTP Bridge Server - Running!                   ║
╠══════════════════════════════════════════════════════════════════════╣
║  Browse MDTP sites in your regular browser                            ║
║  Open: http://127.0.0.1:9999/                                         ║
║  Direct: http://127.0.0.1:9999/127.0.0.1:8585/index.md                ║
╚══════════════════════════════════════════════════════════════════════╝


===========================================================================================
                                6.  I N T E R N A L   W O R K F L O W
===========================================================================================

Flow Diagram:
┌─────────────────────────────┐
│  MDTP Client / Bridge       │
└──────────────┬──────────────┘
               │ Sends Request
               ▼
┌─────────────────────────────┐
│  MDTP Server                │
│  • Parses headers           │
│  • Locates Markdown file    │
│  • Builds MDTP response     │
└──────────────┬──────────────┘
               │ Sends Markdown
               ▼
┌─────────────────────────────┐
│  MDTP Client                │
│  • Extracts body            │
│  • Displays Markdown        │
└─────────────────────────────┘

If the Bridge is used:
   MDTP → Bridge → HTML → Browser View


===========================================================================================
                                7.  D E S I G N   H I G H L I G H T S
===========================================================================================

 • Lightweight and transparent Markdown-first protocol  
 • Fully text-readable request/response exchange  
 • Pure C socket implementation, minimal dependencies  
 • Built-in error handling and status codes  
 • Optional Bridge layer for browsers  
 • Modular design: protocol ↔ transport ↔ rendering  


===========================================================================================
                                8.  U S A G E   S U M M A R Y
===========================================================================================

Start the server:
   ./mdtp server 8585

Fetch a page using client:
   ./mdtp client 127.0.0.1 /index.md

View through your browser via Bridge:
   http://127.0.0.1:9999/127.0.0.1:8585/index.md


===========================================================================================
                                9.  F U T U R E   E X T E N S I O N S
===========================================================================================

 • MDTP Secure (MDTPS) with TLS  
 • Metadata headers (Theme, Author, Version)  
 • Markdown directives for UI components  
 • Native GUI MDTP Browser (“Eclipse”)  
 • OS-level mdtp:// URL registration  


===========================================================================================
                                10.  L I C E N S E
===========================================================================================

MDTP 1.0 is released for educational and experimental use.  
You may modify, extend, or integrate MDTP into non-commercial systems or research tools.

--------------------------------------------------------------------------------------------
                                        EOF  
--------------------------------------------------------------------------------------------
