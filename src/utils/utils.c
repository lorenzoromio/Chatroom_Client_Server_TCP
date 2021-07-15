#include <ctype.h>
#include <string.h>

char *stripString(char *start)
{
    char *end;

    while (isspace(*start))
    {
        ++start;
    }

    end = start + strlen(start) - 1;

    while (end > start && isspace(*end))
    {
        end--;
    }

    end[1] = '\0';

    return start;
}


void str_trim_lf(char *arr, int length)
{
    for (size_t i = 0; i < length; i++)
    { // trim \n
        if (arr[i] == '\n')
        {
            arr[i] = '\0';
            break;
        }
    }
}