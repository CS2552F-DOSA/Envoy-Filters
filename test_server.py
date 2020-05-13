# Reflects the requests from HTTP methods GET, POST, PUT, and DELETE
# Written by Nathan Hamiel (2010)

from BaseHTTPServer import HTTPServer, BaseHTTPRequestHandler
from optparse import OptionParser

import sys

count = 1

class RequestHandler(BaseHTTPRequestHandler):
    
    def do_GET(self):
        global count
        request_path = self.path
        
        print("\n----- Request Start ----->\n")
        print(request_path)
        print(self.headers)
        print("<----- Request End -----\n")
        
        self.send_response(200)
        count += 1
        if sys.argv[1] == '8889':
            self.send_header("cluster", "cluster_1")
        else:
            self.send_header("cluster", "cluster_0")
        self.send_header("fid_timestamp_unix_ns", int(sys.argv[2]))
        self.end_headers()
        self.wfile.write("<html><head><title>12341234</title></head>\n")
        self.wfile.close()
        
    def do_POST(self):
        global count
        request_path = self.path
        
        print("\n----- Request Start ----->\n")
        print(request_path)
        
        request_headers = self.headers
        content_length = request_headers.getheaders('content-length')
        length = int(content_length[0]) if content_length else 0
        
        print(request_headers)
        print(self.rfile.read(length))
        print("<----- Request End -----\n")
        
        self.send_response(200)
        count += 1
        self.send_header("fid_timestamp_unix_ns", str(count))
        self.end_headers()
    
    do_PUT = do_POST
    do_DELETE = do_GET
        
def main():
    port = int(sys.argv[1])
    print('Listening on localhost:%s' % port)
    server = HTTPServer(('', port), RequestHandler)
    server.serve_forever()

        
if __name__ == "__main__":
    parser = OptionParser()
    parser.usage = ("Creates an http-server that will echo out any GET or POST parameters\n"
                    "Run:\n\n"
                    "   reflect")
    (options, args) = parser.parse_args()
    
    main()