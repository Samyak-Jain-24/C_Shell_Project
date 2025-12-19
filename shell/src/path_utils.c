#include <string.h>
#include "path_utils.h"

char* GetCorrectPath(char* str, char* homestr, char* ans){
    int l1 = strlen(str);
    int l2 = strlen(homestr);

    int p1 = 0, p2 = 0;
    while (p1 < l1 && p2 < l2 && str[p1] == homestr[p2]) {
        p1++;
        p2++;
    }

    // If entire home prefix matched → replace with '~'
    if (p2 == l2) {
        int ptr = 0;
        ans[ptr++] = '~';
        while (p1 < l1) {
            ans[ptr++] = str[p1++];
        }
        ans[ptr] = '\0';
    } else {
        // No match, just copy full path
        strcpy(ans, str);
    }

    return ans;
}
