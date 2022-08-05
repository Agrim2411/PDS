#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include "pds.h"
#include "patient.h"

// Load all the patients from a CSV file
int store_patients( char *patient_data_file )
{
	FILE *cfptr;
	char patient_line[500], token;
	struct Patient p, dummy;

	cfptr = (FILE *) fopen(patient_data_file, "r");
	while(fgets(patient_line, sizeof(patient_line)-1, cfptr)){
		//printf("line:%s",patient_line);
		sscanf(patient_line, "%d%s%s", &(p.patient_id),p.patient_name,p.phone);
		print_patient( &p );
		add_patient( &p );
	}
}

void print_patient(struct Patient *p) {
  printf("%d,%s,%s\n", p->patient_id,p->patient_name,p->phone);
}

// Add the given patient into the repository by calling put_rec_by_key
int add_patient(struct Patient *p) {
  int status = put_rec_by_key(p->patient_id, p);
  if(status == PDS_SUCCESS) 
    return PATIENT_SUCCESS;
  else if(status == PDS_REC_ALREADY_PRESENT)
    return PATIENT_DUPLICATE;
  return PATIENT_FAILURE;
}

// Use get_rec_by_key function to retrieve patient
int search_patient( int patient_id, struct Patient *p ) {
  int status = get_rec_by_ndx_key(patient_id, p);
  if(status == PDS_SUCCESS)
    return PATIENT_SUCCESS;
  else if(status == PDS_REC_NOT_FOUND)
    return PATIENT_NOT_FOUND;
  return PATIENT_FAILURE; 
}

// Use get_rec_by_non_ndx_key function to retrieve patient
int search_patient_by_phone(char *phone, struct Patient *p, int *io_count) {
  int status = get_rec_by_non_ndx_key(phone, p, &match_patient_phone, io_count);
  if(status == PDS_SUCCESS)
    return PATIENT_SUCCESS;
  else if(status == PDS_REC_NOT_FOUND)
    return PATIENT_NOT_FOUND;
  return PATIENT_FAILURE;
}

/* Return 0 if phone of the patient matches with phone parameter */
/* Return 1 if phone of the patient does NOT match */
/* Return > 1 in case of any other error */

int match_patient_phone(void *rec, void *key) {
  struct Patient* patient = (struct Patient *)rec; // Store the rec in a struct patient pointer
  char* phoneNumber = (char *)key; // Store the key in a char pointer
  if(!phoneNumber || !patient || !patient->phone) 
    return 2;
  if(strcmp(phoneNumber, patient->phone) == 0)
    return 0; 
  return 1;
}

// Function to delete a record based on ndx_key
int delete_patient(int patient_id) {
  int status = delete_rec_by_ndx_key(patient_id);
  if(status == PDS_SUCCESS)
    return PATIENT_SUCCESS;
  else if(status == PDS_REC_NOT_FOUND || status == PDS_DELETE_FAILED)
    return PATIENT_NOT_FOUND;
  return PATIENT_FAILURE;
}

int overwrite_patient(struct Patient *p) {
  int status = pds_overwrite(p->patient_id, p);
  if(status == PDS_SUCCESS)
    return PATIENT_SUCCESS;
  else if(status == PDS_REC_NOT_FOUND)
    return PATIENT_NOT_FOUND;
  return PATIENT_FAILURE;
}
