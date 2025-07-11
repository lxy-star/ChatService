# 聊天服务器配置示例与通信协议 JSON 标准

---

## 1. 配置服务器示例

```json
{
  "server_ip": "0.0.0.0",
  "server_port": 12345,
  "mysql_host": "127.0.0.1",
  "mysql_user": "root",
  "mysql_password": "123456",
  "mysql_db": "chat_db",
  "redis_host": "127.0.0.1",
  "redis_port": 6379,
  "log_level": "DEBUG" 
}
````

字段说明：

* `server_ip`：监听任意 IP 地址
* `server_port`：自定义端口号
* `mysql_host`：MySQL 数据库地址，通常为本机 127.0.0.1
* `mysql_user`：MySQL 用户名
* `mysql_password`：MySQL 用户密码（建议设置强密码）
* `mysql_db`：数据库名称（需预先创建）
* `redis_host`：Redis 服务地址
* `redis_port`：Redis 服务端口（默认 6379）
* `log_level`：log_level 支持 DEBUG/INFO/WARN/ERROR

---

## 2. 通信协议 JSON 示例

### 2.1 登录请求 `login`

**客户端请求：**

```json
{
  "type": "login",
  "username": "user1",
  "password": "sha256密码哈希"
}
```

**服务器响应：**

```json
{
  "type": "login_result",
  "success": true,
  "reason": ""
}
```

---

### 2.2 注册请求 `register`

**客户端请求：**

```json
{
  "type": "register",
  "username": "user1",
  "password": "sha256密码哈希"
}
```

**服务器响应：**

```json
{
  "type": "register_result",
  "success": true,
  "reason": ""
}
```

---

### 2.3 聊天消息 `chat`

**单聊消息：**

```json
{
  "type": "chat",
  "fromUser": "user1",
  "toUser": "user2",
  "message": "hello, how are you?"
}
```

**广播消息（toUser 为空或 null 表示广播）：**

```json
{
  "type": "chat",
  "fromUser": "user1",
  "toUser": "",
  "message": "hello all"
}
```

---

### 2.4 心跳包 `heartbeat`

**客户端发送：**

```json
{
  "type": "heartbeat",
  "username": "user1"
}
```

**服务器响应：**

```json
{
  "type": "heartbeat_response",
  "status": "ok"
}
```

---

### 2.5 获取历史消息 `history`

**客户端请求最近聊天记录：**

```json
{
  "type": "history",
  "username": "user1",
  "limit": 50
}
```

**服务器响应示例：**

```json
{
  "type": "history_result",
  "messages": [
    {
      "fromUser": "user2",
      "toUser": "user1",
      "message": "hello",
      "sentAt": "2025-07-11 11:00:00"
    },
    {
      "fromUser": "user3",
      "toUser": "",
      "message": "good morning all",
      "sentAt": "2025-07-11 10:50:00"
    }
  ]
}
```

### 2.6 验证用户或群聊是否存在 `verify_user / verify_group`

**客户端请求：**

```json
{
  "type": "verify_user",  // 或 "verify_group"
  "target": "user2"
}
```
**服务器响应：**

```json
{
  "type": "verify_result",
  "target": "user2",
  "exists": true
}
```

### 2.7 请求聊天会话列表 `get_chat_list`

**客户端请求：**

```json
{
  "type": "get_chat_list",
  "username": "user1"
}
```
**服务器响应：**

```json
{
  "type": "chat_list",
  "chats": ["user2", "groupA", "user3"]
}
```

### 2.8 用户登出 `logout`

**客户端请求：**

```json
{
  "type": "logout",
  "username": "user1"
}
```
**服务器响应：**

```json
{
  "type": "logout_result",
  "success": true
}
```

---

## 3. 字段说明

| 字段名        | 类型      | 描述                              |
| ---------- | ------- | ------------------------------- |
| `type`     | string  | 消息类型，如 `login`, `chat` 等        |
| `username` | string  | 用户名                             |
| `password` | string  | 密码的 sha256 哈希值                  |
| `fromUser` | string  | 发送者用户名                          |
| `toUser`   | string  | 接收者用户名，空字符串表示广播                 |
| `message`  | string  | 消息内容                            |
| `success`  | boolean | 操作是否成功                          |
| `reason`   | string  | 错误原因（失败时）                       |
| `status`   | string  | 状态响应，如 `"ok"`                   |
| `sentAt`   | string  | 消息发送时间，格式：`YYYY-MM-DD HH:MM:SS` |
| `limit`    | integer | 请求的历史消息数量限制                     |
