# **Прокси-сервер для логгирования sql запросов**

C++ TCP прокси-сервер для СУБД с возможностью логирования всех SQL запросов, проходящих через него.  

В качестве СУБД использовалась PostgreSQL.  

Прокси может обрабатывать большое количество одновременных соединений. В лог попадают только SQL-запросы. В коде обрабатываются ошибки в протоколе.  

## **Сборка и запуск proxy-server**

### **Запуск запуск proxy-server**

```bash
git clone git@github.com:Hasuk1/Proxy-Server-with-logging-SQL-queries.git
cd src
make proxy_start
```

*`./proxy` 4568 127.0.0.1 5432*

### **Подключение к proxy-server (нужно использовать другой терминал)**

```bash
make connect_to_db
```
*`psql` sslmode=disable host=127.0.0.1 port=4568 user=postgres dbname=postgres*

### **Демонстрация работы**
---
![](misc/%D0%97%D0%B0%D0%BF%D0%B8%D1%81%D1%8C_%D1%8D%D0%BA%D1%80%D0%B0%D0%BD%D0%B0_2023-11-21_%D0%B2_1.47.23_PM.gif)

---