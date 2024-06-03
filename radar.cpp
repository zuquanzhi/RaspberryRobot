#include <ESP8266WiFi.h>

// 定义超声波传感器的引脚，使用GPIO编号
const int trigPin = 5; // D1对应GPIO5
const int echoPin = 4; // D2对应GPIO4

// 定义WiFi的SSID和密码
const char* ssid = "Xiaomi13";
const char* password = "zuwance12345";

// 创建WiFi服务器对象
WiFiServer server(80);

// 定义上次测量时间和测量间隔
unsigned long previousMillis = 0;
const long interval = 1000; // 每隔1秒测量一次

void setup() {
  // 初始化串口
  Serial.begin(115200);

  // 设置超声波传感器的引脚模式
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  // 连接WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nConnected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // 启动服务器
  server.begin();
}

void loop() {
  // 获取当前时间
  unsigned long currentMillis = millis();

  // 如果到了测量时间
  if (currentMillis - previousMillis >= interval) {
    // 记录这次测量的时间
    previousMillis = currentMillis;

    // 读取距离
    long duration, distance;
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    duration = pulseIn(echoPin, HIGH);
    distance = (duration / 2) / 29.1;

    // 打印测量结果到串口监视器
    Serial.print("Distance: ");
    Serial.print(distance);
    Serial.println(" cm");
  }

  // 等待客户端连接
  WiFiClient client = server.available();
  if (client) {
    Serial.println("Client connected!");
    String request = client.readStringUntil('\r');
    Serial.println(request);
    client.flush();

    // 忽略 /favicon.ico 请求
    if (request.indexOf("GET /favicon.ico") != -1) {
      client.stop();
      Serial.println("Favicon request ignored");
      return;
    }

    // 处理HTTP请求
    if (request.indexOf("GET /measure") != -1) {  // 确认请求路径为/measure
      // 读取距离
      long duration, distance;
      digitalWrite(trigPin, LOW);
      delayMicroseconds(2);
      digitalWrite(trigPin, HIGH);
      delayMicroseconds(10);
      digitalWrite(trigPin, LOW);

      duration = pulseIn(echoPin, HIGH);
      distance = (duration / 2) / 29.1;

      // 将距离返回给客户端
      client.print("HTTP/1.1 200 OK\r\n");
      client.print("Content-Type: text/plain\r\n");
      client.print("Connection: close\r\n");
      client.print("\r\n");
      client.print(distance);
      client.print(" cm");
    } else if (request.indexOf("GET / ") != -1) { // 处理根路径
      // 返回一个页面，引导用户访问 /measure
      client.print("HTTP/1.1 200 OK\r\n");
      client.print("Content-Type: text/html\r\n");
      client.print("Connection: close\r\n");
      client.print("\r\n");
      client.print("<html><head><meta http-equiv=\"refresh\" content=\"1\"></head><body><h1>实时距离测量</h1>");
      client.print("<p>当前距离: <span id=\"distance\">加载中...</span></p>");
      client.print("<script>");
      client.print("function fetchDistance() {");
      client.print("  fetch('/measure').then(response => response.text()).then(data => {");
      client.print("    document.getElementById('distance').innerText = data;");
      client.print("  }).catch(error => {");
      client.print("    console.error('Error fetching distance:', error);");
      client.print("  });");
      client.print("}");
      client.print("setInterval(fetchDistance, 1000);");
      client.print("fetchDistance();");
      client.print("</script>");
      client.print("</body></html>");
    } else {
      // 返回404 Not Found
      client.print("HTTP/1.1 404 Not Found\r\n");
      client.print("Content-Type: text/plain\r\n");
      client.print("Connection: close\r\n");
      client.print("\r\n");
      client.print("404 Not Found");
    }

    // 关闭连接
    client.stop();
    Serial.println("Client disconnected!");
  }
}
