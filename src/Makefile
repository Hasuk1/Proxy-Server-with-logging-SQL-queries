CC= g++
CFLAGS= -pthread -std=c++17
CHECKFLAGS= -Wall -Werror -Wextra
SRC= *.cc
DB_TERM= psql
DB_SETTINGS= "sslmode=disable host=127.0.0.1 port=4568 user=postgres dbname=postgres"

all: proxy

proxy:
	$(CC) $(CFLAGS) $(CHECKFLAGS) -o $@ $(SRC)

proxy_start: proxy
	./proxy 4568 127.0.0.1 5432

clean:
	rm -f proxy

clean_log:
	rm -f sql_queries.log

# Запускать в другом терминале!
connect_to_db:
	$(DB_TERM) $(DB_SETTINGS)
