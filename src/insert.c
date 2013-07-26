#include "../include/insert.h"
#include "../include/ra.h"

Insert_t *Insert_make(RA *ra, StrList_t *opt_col_names, Literal_t *values) {
   Insert_t *new_insert = (Insert_t *)calloc(1, sizeof(Insert_t));
   new_insert->ra = ra;
   new_insert->col_names = opt_col_names;
   new_insert->values = values;
   if (!values)
      fprintf(stderr, "Warning: no values given to insert\n");
   
   /* if there are any column names specified, ensure equal cardinality */
   if (opt_col_names) {
      while(true) {
         if (opt_col_names && !values) {
            fprintf(stderr, "Error: more column names specified than values\n");
            return NULL;
         }
         else if (!opt_col_names && values) {
            fprintf(stderr, "Error: more values specified than column names\n");
            return NULL;
         } else if (!opt_col_names && !values) {
            /* then both are the same cardinality, OK */
            break;
         }
         opt_col_names = opt_col_names->next;
         values = values->next;
      }
   }
   return new_insert;
}

void Insert_print(Insert_t *insert) {
   Literal_t *val = insert->values;
   int first = 1;
   printf("Insert ");
   printf("[");
   while (val) {
      if (first) {
         first = 0;
      } else {
         printf(", ");
      }
      Literal_print(val);
      val = val->next;
   }
   printf("] into ");
   RA_print(insert->ra);
   if (insert->col_names) {
      StrList_t *list = insert->col_names;
      first = 1;
      printf(" using columns [");
      while (list) {
         if (first) {
            first = 0;
         } else {
            printf(", ");
         }
         printf("%s", list->str);
         list = list->next;
      }
      printf("]");
   }
   puts("");
}
