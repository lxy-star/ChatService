#进入mysql
mysql -u root -p 

#创建CHAT_DB数据库
CREATE DATABASE chat_db DEFAULT CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci;

#使用chat_db数据库
USE chat_db;

#创建users数据表
CREATE TABLE users (
    id INT AUTO_INCREMENT PRIMARY KEY,
    username VARCHAR(64) NOT NULL UNIQUE,
    password VARCHAR(256) NOT NULL,
    nickname VARCHAR(64),
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

#创建user_sessions数据表
CREATE TABLE user_sessions (
    username VARCHAR(64) NOT NULL PRIMARY KEY,
    session_token VARCHAR(128) NOT NULL,
    login_time DATETIME,
    last_heartbeat DATETIME,
    ip VARCHAR(64),
    is_online BOOLEAN DEFAULT TRUE
);

#创建chat_messages数据表
CREATE TABLE chat_messages (
    id BIGINT AUTO_INCREMENT PRIMARY KEY,
    from_user VARCHAR(64) NOT NULL,
    to_user VARCHAR(64),  -- NULL 表示广播消息
    message TEXT NOT NULL,
    sent_at DATETIME DEFAULT CURRENT_TIMESTAMP
);
