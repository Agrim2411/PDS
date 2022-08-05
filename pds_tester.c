#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<stdbool.h>

#include "pds.h"
#include "patient.h"

int main(int argc, char *argv[]) {
  struct Patient Patient;
  int rec_size = sizeof(struct Patient), status;

  printf("✮✮ Welcome to Pushpanjali Hospital! ✮✮\n\n");
  bool isDataBaseAccessible = true;
  
  if(pds_create("PushpanjaliHospital") != PDS_SUCCESS) {
    printf("\nHospital Databse is currently under maintenance\n\n");
    isDataBaseAccessible = false;
  }
  
  if(isDataBaseAccessible) {
  
    while(true) {  
      printf("Enter your choice as per the menu shown below: \n\n");
      printf("1 - Opening the Hospital Database.\n2 - Add new patient to the hospital.\n3 - Retrieve existing patient records based on patient ID.\n4 - Retrieve existing patient record based on phone number.\n5 - Delete the records of a patient.\n6 - Overwrite the records of an existing patient.\n7 - Close the Hospital Database.\n8 - Exit the Hospital Database.\n");
      int choice;
      scanf("%d", &choice);
      
      if(choice == 1) {
        status = pds_open("PushpanjaliHospital", rec_size);
        if(status == PDS_SUCCESS)
          printf("\nHospital Database has been opened successfully.\n\n");
        else if(status == PDS_REPO_ALREADY_OPEN)
          printf("\nHospital Database is already open.\n\n");
        else 
          printf("\nAn error occured while opening the hospital database.\nPlease try again!\n\n");
      }
    
      else if(choice == 2) {
        char name[50], phone[50], gender[10], disease[50], address[100];
        int id, age;
        double height, weight;
        printf("\nEnter the name of the patient: ");
        scanf("%s", name);
        printf("Enter the ID number of the patient: ");
        scanf("%d", &id);
        printf("Enter the phone number of the patient: ");
        scanf("%s", phone);
        printf("Enter the age of the patient: ");
        scanf("%d", &age);
        printf("Enter the height of the patient: ");
        scanf("%lf", &height);
        printf("Enter the weight of the patient: ");
        scanf("%lf", &weight);
        printf("Enter the gender of the patient: ");
        scanf("%s", gender);
        printf("Enter the disease from which the patient is suffering: ");
        scanf("%s", disease);
        printf("Enter the residance address of the patient: ");
        scanf("%s", address);
        
        Patient.patient_id = id;
        Patient.age = age;
        Patient.height = height;
        Patient.weight = weight;
        strcpy(Patient.patient_name, name);
        strcpy(Patient.phone, phone);
        strcpy(Patient.gender, gender);
        strcpy(Patient.disease, disease);
        strcpy(Patient.address, address);
        
        status = add_patient(&Patient); 
        
        if(status == PATIENT_SUCCESS)
          printf("\nPatient record added successfully.\n\n");
        else if(status == PATIENT_DUPLICATE)
          printf("\nPatient record already exists.\n\n");
        else
          printf("\nAn error occured while adding patient record.\nPlease try again!\n\n");
      }
    
      else if(choice == 3) {
        int id;
        printf("\nEnter the patient id: ");
        scanf("%d", &id);
        status = search_patient(id, &Patient);
        if(status == PATIENT_SUCCESS) {
          printf("\nSearch Successful!\n");
          printf("    Patient Name    - %s\n", Patient.patient_name);
          printf("    Patient Phone   - %s\n", Patient.phone);
          printf("    Patient Id      - %d\n", Patient.patient_id);
          printf("    Patient Age     - %d\n", Patient.age);
          printf("    Patient Height  - %lf\n", Patient.height);
          printf("    Patient Weight  - %lf\n", Patient.weight);
          printf("    Patient Gender  - %s\n", Patient.gender);
          printf("    Patient Disease - %s\n", Patient.disease);
          printf("    Patient Address - %s\n\n", Patient.address);
        }
        else if(status == PATIENT_NOT_FOUND)
          printf("\nPatient is NOT admitted in the hospital!\n\n");
        else
          printf("\nAn error occured while finding patient record.\nPlease try again!\n\n");
      }
    
      else if(choice == 4) {
        char phone[50];
        int actualIO = 0;
        printf("Enter the phone number of the patient: ");
        scanf("%s", phone);
        status = search_patient_by_phone(phone, &Patient, &actualIO);
        if(status == PATIENT_SUCCESS) {
          printf("Search Successful!\n");
          printf("    Patient Name    - %s\n", Patient.patient_name);
          printf("    Patient Phone   - %s\n", Patient.phone);
          printf("    Patient Id      - %d\n", Patient.patient_id);
          printf("    Patient Age     - %d\n", Patient.age);
          printf("    Patient Height  - %lf\n", Patient.height);
          printf("    Patient Weight  - %lf\n", Patient.weight);
          printf("    Patient Gender  - %s\n", Patient.gender);
          printf("    Patient Disease - %s\n", Patient.disease);
          printf("    Patient Address - %s\n\n", Patient.address);
        }
        else if(status == PATIENT_NOT_FOUND)
          printf("Patient is NOT admitted in the hospital!\n\n");
        else
          printf("\nAn error occured while finding patient record.\nPlease try again!\n\n");
      }
    
      else if(choice == 5) {
        printf("Enter the id of the patient to be deleted: ");
        int id;
        scanf("%d", &id);
        status = delete_patient(id);
        if(status == PATIENT_SUCCESS) 
          printf("Patient record successfully deleted.\n\n");
        else if(status == PATIENT_NOT_FOUND)
          printf("Patient is NOT admitted in the hospital!\n\n");
        else
          printf("\nAn error occured while deleting patient record.\n\n");
      }
    
      else if(choice == 6) {
        printf("Enter the id of the patient whose record has to be overwritten: ");
        int id;
        scanf("%d", &id);
        char name[50], phone[50], gender[10], disease[50], address[100];
        int age;
        double weight, height;
        printf("Enter the NEW name of the patient: ");
        scanf("%s", name);
        printf("Enter the NEW phone number of the patient: ");
        scanf("%s", phone);
        printf("Enter the NEW age of the patient: ");
        scanf("%d", &age);
        printf("Enter the NEW height of the patient: ");
        scanf("%lf", &height);
        printf("Enter the NEW weight of the patient: ");
        scanf("%lf", &weight);
        printf("Enter the NEW gender of the patient: ");
        scanf("%s", gender);
        printf("Enter the NEW disease from which the patient is suffering: ");
        scanf("%s", disease);
        printf("Enter the NEW address of the patient: ");
        scanf("%s", address);
        
        Patient.patient_id = id;
        Patient.age = age;
        Patient.height = height;
        Patient.weight = weight;
        strcpy(Patient.patient_name, name);
        strcpy(Patient.phone, phone);
        strcpy(Patient.gender, gender);
        strcpy(Patient.disease, disease);
        strcpy(Patient.address, address);
    
        status = overwrite_patient(&Patient);
        
        if(status == PDS_SUCCESS)
          printf("\nPatient record successfully overwritten.\n\n");
        else if(status == PATIENT_NOT_FOUND)
          printf("\nPatient is NOT admitted in the hospital!\n\n");
        else
          printf("\nAn error occured while overwriting the patient record.\nPlease try again!\n\n");
      }
    
      else if(choice == 7) {
        status = pds_close();
        if(status == PDS_SUCCESS) 
          printf("\nHospital Database has been closed successfully.\n\n");
        else if(status == PDS_REPO_NOT_OPEN)
          printf("\nHospital Database is already closed. You need to open it for furthur usage.\n\n");
        else
          printf("\nAn error occured while closing the hospital database.\nPlease try again!\n\n"); 
      }
      
      else if(choice == 8) {
        status = pds_close();
        if(status == PDS_SUCCESS) {
          printf("\nYou have successfully closed and exited the hospital database!");
          break;
        }
        else if(status == PDS_REPO_NOT_OPEN)
          printf("\nHospital Database is already closed. You need to open it for furthur usage.\n\n");
        else
          printf("\nAn error occured while exiting the hospital database.\nPlease try again!\n\n");
      }
    
      else 
        printf("\nInvalid choice.\n\n");
    }
    
  }
  printf("\n✮✮ Wish you GOOD HEALTH from PUSHPANJALI Hospital! ✮✮\n\n");
  return 0;
}
