#ifndef PATIENT_H
#define PATIENT_H

#define PATIENT_SUCCESS 0
#define PATIENT_FAILURE 1
#define PATIENT_DUPLICATE 2
#define PATIENT_NOT_FOUND 3
 
struct Patient {
  int patient_id, age;
  double height, weight;
  char patient_name[50], phone[50], gender[10], disease[50], address[100];
};

extern struct PDS_RepoInfo *repoHandle;

// Add the given patient into the repository by calling put_rec_by_key
int add_patient( struct Patient *p );

// Overwite existing patient with the given patient pds_overwrite
// Hint: call the PDS function as follows
// pds_overwrite(c->patient_id, c);
int overwrite_patient( struct Patient *p );

// Display patient info in a single line as a CSV without any spaces
void print_patient( struct Patient *p );

// Use get_rec_by_key function to retrieve patient
int search_patient( int patient_id, struct Patient *p );

// Load all the patients from a CSV file
int store_patients( char *patient_data_file );

// Use get_rec_by_non_ndx_key function to retrieve patient
int search_patient_by_phone( char *phone, struct Patient *p, int *io_count );

/* Return 0 if phone of the patient matches with phone parameter */
/* Return 1 if phone of the patient does NOT match */
/* Return > 1 in case of any other error */
int match_patient_phone( void *rec, void *key );

// Function to delete patient by ID
int delete_patient ( int patient_id );

#endif
