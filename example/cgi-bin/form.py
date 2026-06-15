import os
import sys
import urllib.parse

# Read POST body from stdin
content_length = int(os.environ.get('CONTENT_LENGTH', 0))
body = sys.stdin.read(content_length) if content_length > 0 else ''

# Parse the form fields
fields = urllib.parse.parse_qs(body)
name    = fields.get('name', ['stranger'])[0]
message = fields.get('message', [''])[0]

# CGI response: headers first, then blank line, then body
print("Content-Type: text/html\r")
print("\r")
print(f"""<!DOCTYPE html>
<html>
<head><title>CGI response</title></head>
<body>
  <h1>Hello, {name}!</h1>
  <p>{message}</p>
  <p><a href="/">← back</a></p>
  <hr>
  <pre>
SCRIPT_NAME  = {os.environ.get('SCRIPT_NAME', '')}
REQUEST_METHOD = {os.environ.get('REQUEST_METHOD', '')}
CONTENT_LENGTH = {content_length}
  </pre>
</body>
</html>""")
