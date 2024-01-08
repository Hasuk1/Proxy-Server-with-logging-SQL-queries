# **Proxy server for logging sql queries**

C++ TCP proxy server for DBMS with the ability to log all SQL queries passing through it.  

PostgreSQL was used as a DBMS.  

The proxy can handle a large number of simultaneous connections. Only SQL queries are logged. Errors in the protocol are handled in the code.  

## **Build and start proxy-server**

### **Run proxy-server**

```bash
git clone git@github.com:Hasuk1/Proxy-Server-with-logging-SQL-queries.git
cd Proxy-Server-with-logging-SQL-queries/src
make proxy_start
```

*`./proxy` 4568 127.0.0.1 5432*

### **Connect to proxy-server (need to use a different terminal)**

```bash
make connect_to_db
```
*`psql` sslmode=disable host=127.0.0.0.1 port=4568 user=postgres dbname=postgres*

### **Working Demo**
---
![](misc/%D0%97%D0%B0%D0%BF%D0%B8%D1%81%D1%8C_%D1%8D%D0%BA%D1%80%D0%B0%D0%BD%D0%B0_2023-11-21_%D0%B2_1.47.23_PM.gif)

---
