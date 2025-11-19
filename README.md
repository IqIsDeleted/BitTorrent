# BitTorrent

Упрощенная реализация клиента BitTorrent на языке C.

## О клиенте

- **Работает в двух режимах**: Seeder (раздача) и Leecher (загрузка);
- **Поиск Seeder'ов в локальной сети**: Через UDP-Broadcast;
- **Возможность загрузки с нескольких источников**: Через ePoll;
- **Контроль целостности**: Проверка хэшей(SHA1) для каждого фрагмента и для всего файла;
- **Прогресс-бар**: Визуализация процесса загрузки;
- **Создание торрент-файлов**: Отдельное приложение для генерации .torrent файлов;
- **Производительность**: Передача файлов более 1 ГБ без потерь на большой скорости (более 200 мб/с).

## Установка приложения

### Требования
- GCC компилятор
- Библиотека OpenSSL (`libssl-dev`)
- Make

### Установка зависимостей (Ubuntu/Debian)
```bash
sudo apt update
sudo apt install build-essential libssl-dev
```

### Сборка
Сборка основного клиента и приложения для генерации .torrent файлов:
```bash
make
```

Очистка:
```bash
make clean
```

## Использование

### Создание торрент-файлов
```bash
./bin/creator <filename>
```

### Запуск клиента
```bash
./bin/main (-m/--mode) <seed/leech> (-t/--torrent) <torrent_path> (-d/--data) <data_path>
```


## Формат торрент-файла

```C
typedef struct {
  uint8_t infohash[HASH_SIZE]; /**< Torrent file hash for peer discovery */
  uint64_t file_size;          /**< Original file size in bytes */
  char name[NAME_MAX + 1];     /**< Original filename */
  uint32_t piece_size;         /**< Piece size in bytes (64KB) */
  uint32_t pieces_count;       /**< Total number of pieces */
  uint8_t* pieces_hashes;      /**< Array of piece SHA1 hashes (pieces_count *
                                  HASH_SIZE) */
} eltextorrent_file_t;
```