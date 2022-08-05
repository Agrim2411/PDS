#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<string.h>
#include<stdbool.h>

#include "pds.h"
#include "bst.h"

struct PDS_RepoInfo repo_handle;

int pds_create(char *repo_name) {
  char *extendedRepoName = (char *)malloc(30 * sizeof(char));
  strcpy(extendedRepoName, repo_name);
  strcat(extendedRepoName, ".dat");
  FILE *dataFilePointer = fopen(extendedRepoName, "wb+"); // Opening the binary file in write mode.
  char *indexFileName = (char *)malloc(30 * sizeof(char));
  strcpy(indexFileName, repo_name);
  strcat(indexFileName, ".ndx");
  FILE *indexFilePointer = fopen(indexFileName, "wb+");
  if(!dataFilePointer || !indexFilePointer)  // If the file could not be opened successfully, then return file error.
    return PDS_FILE_ERROR;
  fclose(dataFilePointer);  // Closing the file which was opened. 
  fclose(indexFilePointer);
  return PDS_SUCCESS;       // If everything happened successfully then return success.
}

int pds_open(char* repo_name, int rec_size) {
  if(repo_handle.repo_status == PDS_REPO_OPEN || repo_handle.repo_status == PDS_REPO_ALREADY_OPEN)
    return PDS_REPO_ALREADY_OPEN;
  char *extendedRepoName = (char *)malloc(30 * sizeof(char));
  strcpy(extendedRepoName, repo_name);
  strcat(extendedRepoName, ".dat");
  FILE *dataFilePointer = fopen(extendedRepoName, "rb+"); // Open the data file and index file in rb+ mode
  char *indexFileName = (char *)malloc(30 * sizeof(char));
  strcpy(indexFileName, repo_name);
  strcat(indexFileName, ".ndx");
  FILE *indexFilePointer = fopen(indexFileName, "ab+");
  if(!dataFilePointer || !indexFilePointer)        // If the file could not be opened successfully, then return file error.
    return PDS_FILE_ERROR;
  repo_handle.pds_data_fp = dataFilePointer;       // Update the fields of PDS_RepoInfo appropriately
  repo_handle.pds_ndx_fp = indexFilePointer;
  strcpy(repo_handle.pds_name, repo_name);
  repo_handle.repo_status = PDS_REPO_OPEN;
  repo_handle.rec_size = rec_size;
  repo_handle.pds_bst = NULL;
  if(pds_load_ndx() != PDS_SUCCESS)
    return PDS_LOAD_NDX_FAILED;
  fclose(repo_handle.pds_ndx_fp);      // Close only the index file
  return PDS_SUCCESS;
}

int pds_load_ndx(void) {
  int beginOfFile = fseek(repo_handle.pds_ndx_fp, 0, SEEK_SET); // Internal function used by pds_open to read index entries into BST
  if(beginOfFile != 0)
    return PDS_FILE_ERROR;
  struct PDS_NdxInfo* ndxInfo = (struct PDS_NdxInfo*)malloc(sizeof(struct PDS_NdxInfo*));
  while(fread(ndxInfo, sizeof(struct PDS_NdxInfo), 1, repo_handle.pds_ndx_fp)) {
    if(bst_add_node(&repo_handle.pds_bst, ndxInfo->key, ndxInfo) != BST_SUCCESS)  
      return PDS_LOAD_NDX_FAILED;
    ndxInfo = (struct PDS_NdxInfo*)malloc(sizeof(struct PDS_NdxInfo*));
  }
  return PDS_SUCCESS;
}

int put_rec_by_key(int key, void* rec) {
  if(repo_handle.repo_status != PDS_REPO_OPEN)
    return PDS_REPO_NOT_OPEN;
  int endOfFile = fseek(repo_handle.pds_data_fp, 0, SEEK_END); // Seek to the end of the data files - set the file pointer towards the end of the file.
  if(endOfFile != 0)
    return PDS_FILE_ERROR;
  int byteOffset = (int)ftell(repo_handle.pds_data_fp);          // Create an index entry with the current data file location using ftell
  if(byteOffset == -1)
    return PDS_FILE_ERROR;
  struct BST_Node* bstSearchResult = bst_search(repo_handle.pds_bst, key);
  if(bstSearchResult) {
    struct PDS_NdxInfo* ndxInfo = bstSearchResult->data;
    if(!ndxInfo || ndxInfo->is_deleted == 1) {
      if(fseek(repo_handle.pds_data_fp, ndxInfo->offset, SEEK_SET) != 0) // Seek to the file location based on offset in index entry
        return PDS_FILE_ERROR;
      if(fwrite(&key, sizeof(int), 1, repo_handle.pds_data_fp) != 1 && fwrite(rec, repo_handle.rec_size, 1, repo_handle.pds_data_fp) != 1)
        return PDS_ADD_FAILED;
      ndxInfo->is_deleted = 0;
      return PDS_SUCCESS;
    }
  }
  struct PDS_NdxInfo* ndxInfo = (struct PDS_NdxInfo*)malloc(sizeof(struct PDS_NdxInfo));  
  ndxInfo->key = key;
  ndxInfo->offset = byteOffset;
  ndxInfo->is_deleted = 0;   // ENSURE is_deleted is set to 0 when creating index entry.
  int insertIntoBST = bst_add_node(&repo_handle.pds_bst, key, ndxInfo);
  if(insertIntoBST == BST_DUP_KEY) // Add index entry to BST using offset returned by ftell.
    return PDS_REC_ALREADY_PRESENT;
  // Write the key at the current data file location.
  else if(insertIntoBST == BST_NULL)
    return PDS_ADD_FAILED;
  if(fwrite(&key, sizeof(int), 1, repo_handle.pds_data_fp) == 1 && fwrite(rec, repo_handle.rec_size, 1, repo_handle.pds_data_fp) == 1)
    return PDS_SUCCESS; 
  return PDS_ADD_FAILED; 
}

int get_rec_by_ndx_key(int key, void* rec) {
  if(repo_handle.repo_status != PDS_REPO_OPEN)
    return PDS_REPO_NOT_OPEN;
  struct BST_Node* bstSearchResult = bst_search(repo_handle.pds_bst, key); // Search for index entry in BST
  if(!bstSearchResult)  // (NEW) Check if the entry is deleted and if it is deleted, return PDS_REC_NOT_FOUND
    return PDS_REC_NOT_FOUND;
  struct PDS_NdxInfo* ndxInfo = bstSearchResult->data;
  if(!ndxInfo || ndxInfo->is_deleted == 1)
    return PDS_REC_NOT_FOUND;
  int fileLocation = fseek(repo_handle.pds_data_fp, ndxInfo->offset, SEEK_SET);  // Seek to the file location based on offset in index entry
  if(fileLocation != 0)
    return PDS_FILE_ERROR;
  int tempKey;
  if(fread(&tempKey, sizeof(int), 1, repo_handle.pds_data_fp) > 0 && fread(rec, repo_handle.rec_size, 1, repo_handle.pds_data_fp) > 0) // Read the key and record at the file location. 
    return tempKey == key ? PDS_SUCCESS: PDS_REC_NOT_FOUND;
  return PDS_REC_NOT_FOUND;
}

int get_rec_by_non_ndx_key(void *key, void *rec, int (*matcher)(void *rec, void *key), int *io_count) {
  if(repo_handle.repo_status != PDS_REPO_OPEN)
    return PDS_REPO_NOT_OPEN;
  if(fseek(repo_handle.pds_data_fp, 0, SEEK_SET) != 0)  // Seek to beginning of file.
   return PDS_FILE_ERROR;
  // Perform a table scan - iterate over all the records. Read the key and the record
  int temp;
  while(fread(&temp, sizeof(int), 1, repo_handle.pds_data_fp) && fread(rec, repo_handle.rec_size, 1, repo_handle.pds_data_fp)) {
    (*io_count)++;             // Increment io_count by 1 to reflect count no. of records read.
    if(matcher(rec, key) == 0) {  // Use the function in function pointer to compare the record with required key.
      struct BST_Node* bstSearchResult = bst_search(repo_handle.pds_bst, temp);
      if(!bstSearchResult)
        return PDS_REC_NOT_FOUND;
      struct PDS_NdxInfo* ndxInfo = bstSearchResult->data;
      if(!ndxInfo || ndxInfo->is_deleted == 1)  // Check the entry of the record in the BST and see if it is deleted. If so, return PDS_REC_NOT_FOUND
        return PDS_REC_NOT_FOUND;
      return PDS_SUCCESS;      // Return success when record is found.
    }
  }
  return PDS_REC_NOT_FOUND;
}

int delete_rec_by_ndx_key(int key) {
  if(repo_handle.repo_status != PDS_REPO_OPEN)
    return PDS_REPO_NOT_OPEN;
  struct BST_Node* bstSearchResult = bst_search(repo_handle.pds_bst, key);  // Search for the record in the BST using the key
  if(!bstSearchResult)
    return PDS_REC_NOT_FOUND;
  struct PDS_NdxInfo* ndxInfo = bstSearchResult->data;
  if(!ndxInfo || ndxInfo->is_deleted == 1)  // If record not found, or if it has already been deleted, return PDS_DELETE_FAILED 
    return PDS_DELETE_FAILED;
  ndxInfo->is_deleted = 1;
  return PDS_SUCCESS;
}

int pds_overwrite(int key, void *rec) {
  if(repo_handle.repo_status != PDS_REPO_OPEN)
    return PDS_REPO_NOT_OPEN;
  struct BST_Node* bstSearchResult = bst_search(repo_handle.pds_bst, key);
  if(!bstSearchResult)
    return PDS_REC_NOT_FOUND;
  struct PDS_NdxInfo* ndxInfo = bstSearchResult->data;
  if(!ndxInfo || ndxInfo->is_deleted == 1)
    return PDS_REC_NOT_FOUND;;
  if(fseek(repo_handle.pds_data_fp, ndxInfo->offset, SEEK_SET) != 0) // Seek to the file location based on offset in index entry
    return PDS_FILE_ERROR;
  if(fwrite(&key, sizeof(int), 1, repo_handle.pds_data_fp) == 1 && fwrite(rec, repo_handle.rec_size, 1, repo_handle.pds_data_fp) == 1)
    return PDS_SUCCESS;
  return PDS_UPDATE_FAILED;
}

bool writeFromBSTtoIndexFile(struct BST_Node *node) { // Preorder traversal
  if(!node)
    return true;
  struct PDS_NdxInfo* ndxInfo = node->data;
  if(ndxInfo->is_deleted == 0 && fwrite(ndxInfo, sizeof(struct PDS_NdxInfo), 1, repo_handle.pds_ndx_fp) != 1) // Ignore the index entries that have already been deleted. 
    return false;
  return writeFromBSTtoIndexFile(node->left_child) && writeFromBSTtoIndexFile(node->right_child);
}

int pds_close() {
  if(repo_handle.repo_status != PDS_REPO_OPEN)
    return PDS_REPO_NOT_OPEN;
  char *indexFileName = (char*)malloc(30 * sizeof(char));
  strcpy(indexFileName, repo_handle.pds_name);
  strcat(indexFileName, ".ndx");
  FILE* indexFilePointer = fopen(indexFileName, "wb");   // Open the index file in wb mode (write mode, not append mode)
  if(!indexFilePointer)
    return PDS_FILE_ERROR;
  repo_handle.pds_ndx_fp = indexFilePointer;
  if(!writeFromBSTtoIndexFile(repo_handle.pds_bst)) // Unload the BST into the index file by traversing it in PRE-ORDER (overwrite the entire index file)
    return PDS_NDX_SAVE_FAILED;
  bst_destroy(repo_handle.pds_bst);             // Free the BST by calling bst_destroy()
  fclose(repo_handle.pds_ndx_fp);               // Close the index file and data file
  fclose(repo_handle.pds_data_fp);               
  repo_handle.repo_status = PDS_REPO_CLOSED;    // update repo_status
  return PDS_SUCCESS;                           // return PDS_SUCCESS
}
