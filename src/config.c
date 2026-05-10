#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "config.h"

static int is_valid_buffer(char bufferName)
{
    return bufferName == 'A' || bufferName == 'B';
}

void loadConfig(const char *filename, Config *config)
{
    // Varsayılan değerler
    config->bufferASize = 5;
    config->bufferBSize = 5;
    config->runtime = 60;
    config->deadlockMode = 0;
    config->producerCount = 0;
    config->consumerCount = 0;

    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        printf("Config file (%s) could not be opened. Using default values.\n", filename);
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), file))
    {
        // Yorum satırlarını (// sonrası) yoksay
        char *comment = strstr(line, "//");
        if (comment)
            *comment = '\0';

        int size, time, id;
        char buf1, buf2;

        // Buffer A ve B Boyutları: Örn: A[22]
        if (sscanf(line, "A[%d]", &size) == 1)
        {
            if (size > 0)
            {
                config->bufferASize = size;
            }
            else
            {
                printf("Invalid buffer size for A. Using default value.\n");
            }
        }
        else if (sscanf(line, "B[%d]", &size) == 1)
        {
            if (size > 0)
            {
                config->bufferBSize = size;
            }
            else
            {
                printf("Invalid buffer size for B. Using default value.\n");
            }
        }
        // Çalışma Süresi: Örn: t:20
        else if (sscanf(line, "t:%d", &time) == 1)
        {
            if (time > 0)
            {
                config->runtime = time;
            }
            else
            {
                printf("Invalid runtime. Using default value.\n");
            }
        }
        // Üretici Yönlendirmesi: Örn: P1>A
        else if (sscanf(line, "P%d>%c", &id, &buf1) == 2)
        {
            if (!is_valid_buffer(buf1))
            {
                printf("Invalid buffer name for producer P%d: %c\n", id, buf1);
                continue;
            }

            if (config->producerCount >= 50)
            {
                printf("Too many producers. Maximum producer count is 50.\n");
                continue;
            }

            config->producers[config->producerCount].id = id;
            config->producers[config->producerCount].outBuffer = buf1;
            config->producers[config->producerCount].inBuffer = 0;
            config->producers[config->producerCount].sleepTime = 10;
            config->producerCount++;
        }
        // Tüketici - Dairesel Bağımlılık (Hem okur hem yazar): Örn: B>C3>A
        else if (sscanf(line, "%c>C%d>%c", &buf1, &id, &buf2) == 3)
        {
            if (!is_valid_buffer(buf1) || !is_valid_buffer(buf2))
            {
                printf("Invalid buffer name for consumer C%d: %c -> %c\n", id, buf1, buf2);
                continue;
            }

            if (config->consumerCount >= 50)
            {
                printf("Too many consumers. Maximum consumer count is 50.\n");
                continue;
            }

            config->consumers[config->consumerCount].id = id;
            config->consumers[config->consumerCount].inBuffer = buf1;
            config->consumers[config->consumerCount].outBuffer = buf2;
            config->consumers[config->consumerCount].sleepTime = 10;
            config->consumerCount++;
        }
        // Normal Tüketici: Örn: A>C1
        else if (sscanf(line, "%c>C%d", &buf1, &id) == 2)
        {
            if (!is_valid_buffer(buf1))
            {
                printf("Invalid buffer name for consumer C%d: %c\n", id, buf1);
                continue;
            }

            if (config->consumerCount >= 50)
            {
                printf("Too many consumers. Maximum consumer count is 50.\n");
                continue;
            }

            config->consumers[config->consumerCount].id = id;
            config->consumers[config->consumerCount].inBuffer = buf1;
            config->consumers[config->consumerCount].outBuffer = 0;
            config->consumers[config->consumerCount].sleepTime = 10;
            config->consumerCount++;
        }
        // Üretici Uyku Süresi: Örn: P1:2
        else if (sscanf(line, "P%d:%d", &id, &time) == 2)
        {
            for (int i = 0; i < config->producerCount; i++)
            {
                if (config->producers[i].id == id)
                    config->producers[i].sleepTime = time;
            }
        }
        // Tüketici Uyku Süresi: Örn: C1:2
        else if (sscanf(line, "C%d:%d", &id, &time) == 2)
        {
            for (int i = 0; i < config->consumerCount; i++)
            {
                if (config->consumers[i].id == id)
                    config->consumers[i].sleepTime = time;
            }
        }
        // Deadlock Modu için kendi eklememiz (Opsiyonel)
        else if (sscanf(line, "deadlock_mode=%d", &time) == 1)
        {
            config->deadlockMode = time;
        }
    }
    fclose(file);

    printf("Configuration loaded. Producers: %d, Consumers: %d, Runtime: %ds\n",
           config->producerCount, config->consumerCount, config->runtime);
}