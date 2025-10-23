/*********************************************************************
 * HTTPX                                                             *
 *                                                                   *
 * A simple command-line tool to make HTTP requests and generate     *
 * code snippets in multiple languages (cURL, JavaScript, Python,    *
 * Rust, Java).                                                      *
 *                                                                   *
 * Features: Custom headers, JSON formatting, response timing,       *
 *           multiline body input, code generation                   *
 *                                                                   *
 * Compile: gcc -o httpx httpx.c -lcurl                              *
 * Run:     ./httpx                                                  *
 *                                                                   *
 * Dependencies: libcurl                                             *
 *   Ubuntu/Debian: sudo apt-get install libcurl4-openssl-dev        *
 *   Fedora/RHEL:   sudo dnf install libcurl-devel                   *
 *   macOS:         brew install curl                                *
 *********************************************************************/

 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include <curl/curl.h>
 #include <time.h>
 
 #define MAX_HEADERS 50
 #define MAX_HEADER_LEN 512
 #define MAX_URL_LEN 2048
 #define MAX_BODY_LEN 16384
 #define VERSION "1.0.0"
 
 #define COLOR_RESET "\033[0m"
 #define COLOR_BOLD "\033[1m"
 #define COLOR_RED "\033[31m"
 #define COLOR_GREEN "\033[32m"
 #define COLOR_YELLOW "\033[33m"
 #define COLOR_BLUE "\033[34m"
 #define COLOR_MAGENTA "\033[35m"
 #define COLOR_CYAN "\033[36m"
 
 typedef struct {
     char *memory;
     size_t size;
 } ResponseData;
 
 typedef struct {
     char url[MAX_URL_LEN];
     char method[16];
     char headers[MAX_HEADERS][MAX_HEADER_LEN];
     int header_count;
     char body[MAX_BODY_LEN];
     int follow_redirects;
     int verbose;
     long timeout;
     char auth[256];
 } Request;
 
 static size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp) {
     size_t realsize = size * nmemb;
     ResponseData *mem = (ResponseData *)userp;
 
     char *ptr = realloc(mem->memory, mem->size + realsize + 1);
     if(!ptr) {
         printf("Not enough memory!\n");
         return 0;
     }
 
     mem->memory = ptr;
     memcpy(&(mem->memory[mem->size]), contents, realsize);
     mem->size += realsize;
     mem->memory[mem->size] = 0;
 
     return realsize;
 }
 
 void print_banner() {
     printf("\n");
     printf("%s╔═════════════════════════════════════════════════╗%s\n", COLOR_CYAN, COLOR_RESET);
     printf("%s║      %sHTTPX%s - Simple HTTP Client CLI v%s      ║%s\n", 
            COLOR_CYAN, COLOR_BOLD, COLOR_CYAN, VERSION, COLOR_RESET);
     printf("%s╚═════════════════════════════════════════════════╝%s\n", COLOR_CYAN, COLOR_RESET);
     printf("\n");
 }
 
 void print_menu() {
     printf("%s┌─ Main Menu ─────────────────────────────────────┐%s\n", COLOR_BLUE, COLOR_RESET);
     printf("%s│%s  1. New Request                                 %s│%s\n", COLOR_BLUE, COLOR_RESET, COLOR_BLUE, COLOR_RESET);
     printf("%s│%s  2. View Last Request                           %s│%s\n", COLOR_BLUE, COLOR_RESET, COLOR_BLUE, COLOR_RESET);
     printf("%s│%s  3. Generate Code                               %s│%s\n", COLOR_BLUE, COLOR_RESET, COLOR_BLUE, COLOR_RESET);
     printf("%s│%s  4. Help                                        %s│%s\n", COLOR_BLUE, COLOR_RESET, COLOR_BLUE, COLOR_RESET);
     printf("%s│%s  5. Exit                                        %s│%s\n", COLOR_BLUE, COLOR_RESET, COLOR_BLUE, COLOR_RESET);
     printf("%s└─────────────────────────────────────────────────┘%s\n\n", COLOR_BLUE, COLOR_RESET);
 }
 
 void format_json(const char *json_str);
 
 void generate_curl_code(Request *req) {
     printf("\n%s=== cURL ===%s\n", COLOR_GREEN, COLOR_RESET);
     printf("curl -X %s '%s'", req->method, req->url);
 
     for(int i = 0; i < req->header_count; i++) {
         printf(" \\\n  -H '%s'", req->headers[i]);
     }
 
     if(strlen(req->body) > 0) {
         printf(" \\\n  -d '%s'", req->body);
     }
 
     if(req->follow_redirects) {
         printf(" \\\n  -L");
     }
 
     if(req->timeout > 0) {
         printf(" \\\n  --max-time %ld", req->timeout);
     }
     printf("\n\n");
 }
 
 void generate_javascript_code(Request *req) {
     printf("%s=== JavaScript (Fetch API) ===%s\n", COLOR_GREEN, COLOR_RESET);
     printf("fetch('%s', {\n", req->url);
     printf("  method: '%s',\n", req->method);
 
     if(req->header_count > 0) {
         printf("  headers: {\n");
         for(int i = 0; i < req->header_count; i++) {
             char header_copy[MAX_HEADER_LEN];
             strcpy(header_copy, req->headers[i]);
             char *colon = strchr(header_copy, ':');
             if(colon) {
                 *colon = '\0';
                 char *value = colon + 1;
                 while(*value == ' ') value++;
                 printf("    '%s': '%s'", header_copy, value);
                 if(i < req->header_count - 1) printf(",");
                 printf("\n");
             }
         }
         printf("  },\n");
     }
 
     if(strlen(req->body) > 0) {
 
         if(req->body[0] == '{' || req->body[0] == '[') {
             printf("  body: JSON.stringify(%s)\n", req->body);
         } else {
             printf("  body: '%s'\n", req->body);
         }
     }
 
     printf("})\n");
     printf("  .then(response => response.json())\n");
     printf("  .then(data => console.log(data))\n");
     printf("  .catch(error => console.error('Error:', error));\n\n");
 }
 
 void generate_python_code(Request *req) {
     printf("%s=== Python (requests) ===%s\n", COLOR_GREEN, COLOR_RESET);
     printf("import requests\nimport json\n\n");
     printf("url = '%s'\n", req->url);
 
     if(req->header_count > 0) {
         printf("headers = {\n");
         for(int i = 0; i < req->header_count; i++) {
             char header_copy[MAX_HEADER_LEN];
             strcpy(header_copy, req->headers[i]);
             char *colon = strchr(header_copy, ':');
             if(colon) {
                 *colon = '\0';
                 char *value = colon + 1;
                 while(*value == ' ') value++;
                 printf("    '%s': '%s'", header_copy, value);
                 if(i < req->header_count - 1) printf(",");
                 printf("\n");
             }
         }
         printf("}\n\n");
     }
 
     if(strlen(req->body) > 0) {
         printf("data = '''%s'''\n\n", req->body);
     }
 
     printf("response = requests.%s(url", 
            strcmp(req->method, "GET") == 0 ? "get" :
            strcmp(req->method, "POST") == 0 ? "post" :
            strcmp(req->method, "PUT") == 0 ? "put" :
            strcmp(req->method, "DELETE") == 0 ? "delete" : "request");
 
     if(req->header_count > 0) printf(", headers=headers");
     if(strlen(req->body) > 0) printf(", data=data");
     printf(")\n");
     printf("print(response.json())\n\n");
 }
 
 void generate_rust_code(Request *req) {
     printf("%s=== Rust (reqwest) ===%s\n", COLOR_GREEN, COLOR_RESET);
     printf("use reqwest;\n\n");
     printf("#[tokio::main]\n");
     printf("async fn main() -> Result<(), Box<dyn std::error::Error>> {\n");
     printf("    let client = reqwest::Client::new();\n");
 
     if(strlen(req->body) > 0) {
         printf("    let body = r#\"%s\"#;\n\n", req->body);
     }
 
     printf("    let response = client.%s(\"%s\")\n", 
            strcmp(req->method, "GET") == 0 ? "get" :
            strcmp(req->method, "POST") == 0 ? "post" :
            strcmp(req->method, "PUT") == 0 ? "put" :
            strcmp(req->method, "DELETE") == 0 ? "delete" : "request", req->url);
 
     for(int i = 0; i < req->header_count; i++) {
         char header_copy[MAX_HEADER_LEN];
         strcpy(header_copy, req->headers[i]);
         char *colon = strchr(header_copy, ':');
         if(colon) {
             *colon = '\0';
             char *value = colon + 1;
             while(*value == ' ') value++;
             printf("        .header(\"%s\", \"%s\")\n", header_copy, value);
         }
     }
 
     if(strlen(req->body) > 0) {
         printf("        .body(body)\n");
     }
 
     printf("        .send()\n");
     printf("        .await?;\n\n");
     printf("    let body = response.text().await?;\n");
     printf("    println!(\"{}\", body);\n");
     printf("    Ok(())\n");
     printf("}\n\n");
 }
 
 void generate_java_code(Request *req) {
     printf("%s=== Java (HttpClient) ===%s\n", COLOR_GREEN, COLOR_RESET);
     printf("import java.net.URI;\n");
     printf("import java.net.http.HttpClient;\n");
     printf("import java.net.http.HttpRequest;\n");
     printf("import java.net.http.HttpResponse;\n\n");
     printf("public class HttpExample {\n");
     printf("    public static void main(String[] args) throws Exception {\n");
     printf("        HttpClient client = HttpClient.newHttpClient();\n");
 
     if(strlen(req->body) > 0) {
         printf("        String jsonBody = \"\"\"\n");
         printf("            %s\n", req->body);
         printf("            \"\"\";\n\n");
     }
 
     printf("        HttpRequest.Builder builder = HttpRequest.newBuilder()\n");
     printf("            .uri(URI.create(\"%s\"))\n", req->url);
 
     for(int i = 0; i < req->header_count; i++) {
         char header_copy[MAX_HEADER_LEN];
         strcpy(header_copy, req->headers[i]);
         char *colon = strchr(header_copy, ':');
         if(colon) {
             *colon = '\0';
             char *value = colon + 1;
             while(*value == ' ') value++;
             printf("            .header(\"%s\", \"%s\")\n", header_copy, value);
         }
     }
 
     if(strlen(req->body) > 0) {
         printf("            .%s(HttpRequest.BodyPublishers.ofString(jsonBody));\n",
                strcmp(req->method, "GET") == 0 ? "GET" :
                strcmp(req->method, "POST") == 0 ? "POST" :
                strcmp(req->method, "PUT") == 0 ? "PUT" :
                strcmp(req->method, "DELETE") == 0 ? "DELETE" : "method");
     } else {
         printf("            .%s(HttpRequest.BodyPublishers.noBody());\n", req->method);
     }
 
     printf("\n        HttpRequest request = builder.build();\n");
     printf("        HttpResponse<String> response = client.send(request,\n");
     printf("            HttpResponse.BodyHandlers.ofString());\n");
     printf("        System.out.println(response.body());\n");
     printf("    }\n");
     printf("}\n\n");
 }
 
 void generate_code_menu(Request *req) {
     int choice;
     printf("\n%s┌─ Generate Code ─────────────────────────────────┐%s\n", COLOR_MAGENTA, COLOR_RESET);
     printf("%s│%s  1. cURL                                        %s│%s\n", COLOR_MAGENTA, COLOR_RESET, COLOR_MAGENTA, COLOR_RESET);
     printf("%s│%s  2. JavaScript (Fetch)                          %s│%s\n", COLOR_MAGENTA, COLOR_RESET, COLOR_MAGENTA, COLOR_RESET);
     printf("%s│%s  3. Python (requests)                           %s│%s\n", COLOR_MAGENTA, COLOR_RESET, COLOR_MAGENTA, COLOR_RESET);
     printf("%s│%s  4. Rust (reqwest)                              %s│%s\n", COLOR_MAGENTA, COLOR_RESET, COLOR_MAGENTA, COLOR_RESET);
     printf("%s│%s  5. Java (HttpClient)                           %s│%s\n", COLOR_MAGENTA, COLOR_RESET, COLOR_MAGENTA, COLOR_RESET);
     printf("%s│%s  6. All Languages                               %s│%s\n", COLOR_MAGENTA, COLOR_RESET, COLOR_MAGENTA, COLOR_RESET);
     printf("%s└─────────────────────────────────────────────────┘%s\n", COLOR_MAGENTA, COLOR_RESET);
     printf("\nSelect language: ");
     scanf("%d", &choice);
     getchar();
 
     switch(choice) {
         case 1: generate_curl_code(req); break;
         case 2: generate_javascript_code(req); break;
         case 3: generate_python_code(req); break;
         case 4: generate_rust_code(req); break;
         case 5: generate_java_code(req); break;
         case 6:
             generate_curl_code(req);
             generate_javascript_code(req);
             generate_python_code(req);
             generate_rust_code(req);
             generate_java_code(req);
             break;
         default:
             printf("%s[!] Invalid choice%s\n", COLOR_RED, COLOR_RESET);
     }
 }
 
 void execute_request(Request *req) {
     CURL *curl;
     CURLcode res;
     ResponseData response = {0};
     struct curl_slist *headers_list = NULL;
     clock_t start, end;
 
     response.memory = malloc(1);
     response.size = 0;
 
     curl_global_init(CURL_GLOBAL_DEFAULT);
     curl = curl_easy_init();
 
     if(curl) {
         printf("\n%s[→] Sending %s request to %s...%s\n", 
                COLOR_YELLOW, req->method, req->url, COLOR_RESET);
 
         start = clock();
 
         curl_easy_setopt(curl, CURLOPT_URL, req->url);
         curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, req->method);
         curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
         curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&response);
 
         for(int i = 0; i < req->header_count; i++) {
             headers_list = curl_slist_append(headers_list, req->headers[i]);
         }
 
         if(headers_list) {
             curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers_list);
         }
 
         if(strlen(req->body) > 0) {
             curl_easy_setopt(curl, CURLOPT_POSTFIELDS, req->body);
             curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)strlen(req->body));
         }
 
         if(req->follow_redirects) {
             curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
         }
 
         if(req->timeout > 0) {
             curl_easy_setopt(curl, CURLOPT_TIMEOUT, req->timeout);
         }
 
         if(req->verbose) {
             curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
         }
 
         res = curl_easy_perform(curl);
         end = clock();
 
         if(res != CURLE_OK) {
             printf("%s[✗] Request failed: %s%s\n", 
                    COLOR_RED, curl_easy_strerror(res), COLOR_RESET);
         } else {
             long response_code;
             curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
             double elapsed = ((double)(end - start)) / CLOCKS_PER_SEC * 1000;
 
             printf("%s[✓] Response received in %.2fms%s\n", 
                    COLOR_GREEN, elapsed, COLOR_RESET);
             printf("%s[i] Status Code: %ld%s\n", 
                    response_code >= 200 && response_code < 300 ? COLOR_GREEN : COLOR_RED,
                    response_code, COLOR_RESET);
             printf("\n%s--- Response Body ---%s\n", COLOR_CYAN, COLOR_RESET);
 
             if(response.memory && (response.memory[0] == '{' || response.memory[0] == '[')) {
                 format_json(response.memory);
             } else {
                 printf("%s\n", response.memory);
             }
             printf("%s---------------------%s\n\n", COLOR_CYAN, COLOR_RESET);
         }
 
         curl_slist_free_all(headers_list);
         curl_easy_cleanup(curl);
     }
 
     free(response.memory);
     curl_global_cleanup();
 }
 
 void show_help() {
     printf("\n%s=== HTTPX Help ===%s\n\n", COLOR_CYAN, COLOR_RESET);
     printf("%sFeatures:%s\n", COLOR_BOLD, COLOR_RESET);
     printf("  • Support for all HTTP methods (GET, POST, PUT, DELETE, PATCH, etc.)\n");
     printf("  • Custom headers support\n");
     printf("  • Multiline JSON/body input\n");
     printf("  • Follow redirects\n");
     printf("  • Request timeout\n");
     printf("  • Code generation for multiple languages\n");
     printf("  • Colored and formatted output\n");
     printf("  • Response time measurement\n\n");
 
     printf("%sUsage:%s\n", COLOR_BOLD, COLOR_RESET);
     printf("  1. Select 'New Request' from the menu\n");
     printf("  2. Enter request details (URL, method, headers, body)\n");
     printf("  3. For JSON body: Type or paste (multiline supported), end with @@@ on new line\n");
     printf("  4. View the response\n");
     printf("  5. Generate code snippets in various languages\n\n");
 
     printf("%sTips:%s\n", COLOR_BOLD, COLOR_RESET);
     printf("  • For POST/PUT with JSON, add header: Content-Type: application/json\n");
     printf("  • Use @@@ on a new line to finish multiline body input\n\n");
 }
 
 void read_multiline_body(char *body, size_t max_len) {
     printf("\nEnter request body (multiline supported, end with @@@ on new line):\n");
     size_t total = 0;
     char line[1024];
     int first_line = 1;
 
     while(total < max_len - 1 && fgets(line, sizeof(line), stdin)) {
 
         if(strcmp(line, "@@@\n") == 0 || strcmp(line, "@@@\r\n") == 0) {
             break;
         }
 
         size_t len = strlen(line);
         if(total + len < max_len - 1) {
 
             if(!first_line && total < max_len - 2) {
                 body[total++] = '\n';
             }
             first_line = 0;
 
             if(len > 0 && line[len-1] == '\n') {
                 line[len-1] = '\0';
                 len--;
             }
             if(len > 0 && line[len-1] == '\r') {
                 line[len-1] = '\0';
                 len--;
             }
 
             strcpy(body + total, line);
             total += len;
         }
     }
 
     body[total] = '\0';
 }
 
 void format_json(const char *json_str) {
     int indent = 0;
     int in_string = 0;
     char prev_char = 0;
 
     for(size_t i = 0; i < strlen(json_str); i++) {
         char c = json_str[i];
 
         if(c == '"' && prev_char != '\\') {
             in_string = !in_string;
             putchar(c);
         }
 
         else if(in_string) {
             putchar(c);
         }
 
         else if(c == '{' || c == '[') {
             putchar(c);
             indent++;
             if(i + 1 < strlen(json_str) && json_str[i+1] != '}' && json_str[i+1] != ']') {
                 printf("\n%*s", indent * 2, "");
             }
         }
         else if(c == '}' || c == ']') {
             indent--;
 
             if(prev_char != '{' && prev_char != '[') {
                 printf("\n%*s", indent * 2, "");
             }
             putchar(c);
         }
         else if(c == ',') {
             putchar(c);
             printf("\n%*s", indent * 2, "");
         }
         else if(c == ':') {
             printf(": ");
         }
         else if(c != ' ' && c != '\n' && c != '\r' && c != '\t') {
             putchar(c);
         }
 
         if(c != ' ' && c != '\n' && c != '\r' && c != '\t') {
             prev_char = c;
         }
     }
     printf("\n");
 }
 
 void configure_request(Request *req) {
     char input[512];
 
     printf("\n%s=== Configure Request ===%s\n", COLOR_CYAN, COLOR_RESET);
 
     printf("\nEnter URL: ");
     fgets(req->url, MAX_URL_LEN, stdin);
     req->url[strcspn(req->url, "\n")] = 0;
 
     printf("Enter Method (GET/POST/PUT/DELETE/PATCH) [GET]: ");
     fgets(req->method, sizeof(req->method), stdin);
     req->method[strcspn(req->method, "\n")] = 0;
     if(strlen(req->method) == 0) strcpy(req->method, "GET");
 
     for(int i = 0; req->method[i]; i++) {
         if(req->method[i] >= 'a' && req->method[i] <= 'z') {
             req->method[i] = req->method[i] - 32;
         }
     }
 
     printf("\nAdd headers? (y/n) [n]: ");
     fgets(input, sizeof(input), stdin);
     req->header_count = 0;
     if(input[0] == 'y' || input[0] == 'Y') {
         printf("Enter headers (format: Key: Value, empty line to finish):\n");
         while(req->header_count < MAX_HEADERS) {
             printf("  Header %d: ", req->header_count + 1);
             fgets(req->headers[req->header_count], MAX_HEADER_LEN, stdin);
             req->headers[req->header_count][strcspn(req->headers[req->header_count], "\n")] = 0;
             if(strlen(req->headers[req->header_count]) == 0) break;
             req->header_count++;
         }
     }
 
     if(strcmp(req->method, "GET") != 0 && strcmp(req->method, "DELETE") != 0) {
         printf("\nAdd request body? (y/n) [n]: ");
         fgets(input, sizeof(input), stdin);
         if(input[0] == 'y' || input[0] == 'Y') {
             read_multiline_body(req->body, MAX_BODY_LEN);
 
             if(req->body[0] == '{' || req->body[0] == '[') {
                 int has_content_type = 0;
                 for(int i = 0; i < req->header_count; i++) {
                     if(strncasecmp(req->headers[i], "Content-Type:", 13) == 0) {
                         has_content_type = 1;
                         break;
                     }
                 }
                 if(!has_content_type && req->header_count < MAX_HEADERS) {
                     strcpy(req->headers[req->header_count], "Content-Type: application/json");
                     req->header_count++;
                     printf("%s[i] Auto-added Content-Type: application/json header%s\n", 
                            COLOR_YELLOW, COLOR_RESET);
                 }
             }
         } else {
             req->body[0] = '\0';
         }
     } else {
         req->body[0] = '\0';
     }
 
     printf("\nFollow redirects? (y/n) [y]: ");
     fgets(input, sizeof(input), stdin);
     req->follow_redirects = (input[0] != 'n' && input[0] != 'N');
 
     printf("Timeout in seconds (0 for none) [0]: ");
     fgets(input, sizeof(input), stdin);
     req->timeout = atol(input);
 
     printf("Verbose mode? (y/n) [n]: ");
     fgets(input, sizeof(input), stdin);
     req->verbose = (input[0] == 'y' || input[0] == 'Y');
 }
 
 void view_request(Request *req) {
     printf("\n%s=== Last Request ===%s\n", COLOR_CYAN, COLOR_RESET);
     printf("%sURL:%s %s\n", COLOR_BOLD, COLOR_RESET, req->url);
     printf("%sMethod:%s %s\n", COLOR_BOLD, COLOR_RESET, req->method);
 
     if(req->header_count > 0) {
         printf("%sHeaders:%s\n", COLOR_BOLD, COLOR_RESET);
         for(int i = 0; i < req->header_count; i++) {
             printf("  %s\n", req->headers[i]);
         }
     }
 
     if(strlen(req->body) > 0) {
         printf("%sBody:%s\n%s\n", COLOR_BOLD, COLOR_RESET, req->body);
     }
 
     printf("%sFollow Redirects:%s %s\n", COLOR_BOLD, COLOR_RESET, 
            req->follow_redirects ? "Yes" : "No");
     printf("%sTimeout:%s %ld seconds\n", COLOR_BOLD, COLOR_RESET, req->timeout);
     printf("\n");
 }
 
 int main() {
     Request last_request = {0};
     int choice;
 
     strcpy(last_request.method, "GET");
     last_request.follow_redirects = 1;
 
     print_banner();
 
     while(1) {
         print_menu();
         printf("Select option: ");
         scanf("%d", &choice);
         getchar();
 
         switch(choice) {
             case 1:
                 configure_request(&last_request);
                 execute_request(&last_request);
                 printf("\nPress Enter to continue...");
                 getchar();
                 break;
             case 2:
                 if(strlen(last_request.url) == 0) {
                     printf("%s[!] No request made yet%s\n", COLOR_RED, COLOR_RESET);
                 } else {
                     view_request(&last_request);
                 }
                 printf("\nPress Enter to continue...");
                 getchar();
                 break;
             case 3:
                 if(strlen(last_request.url) == 0) {
                     printf("%s[!] No request to generate code from%s\n", COLOR_RED, COLOR_RESET);
                 } else {
                     generate_code_menu(&last_request);
                 }
                 printf("\nPress Enter to continue...");
                 getchar();
                 break;
             case 4:
                 show_help();
                 printf("\nPress Enter to continue...");
                 getchar();
                 break;
             case 5:
                 printf("\n%s[✓] Thanks for using HTTPX!%s\n\n", COLOR_GREEN, COLOR_RESET);
                 return 0;
             default:
                 printf("%s[!] Invalid option%s\n", COLOR_RED, COLOR_RESET);
                 printf("\nPress Enter to continue...");
                 getchar();
         }
     }
 
     return 0;
 }
