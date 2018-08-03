/*==========================================================================*
 *                                                                          *
 * Licensed under the Apache License, Version 2.0 (the "License");          *
 * you may not use this file except in compliance with the License.         *
 * You may obtain a copy of the License at                                  *
 *                                                                          *
 *                http://www.apache.org/licenses/LICENSE-2.0                *
 *                                                                          *
 * Unless required by applicable law or agreed to in writing, software      *
 * distributed under the License is distributed on an "AS IS" BASIS,        *
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. *
 * See the License for the specific language governing permissions and      *
 * limitations under the License.                                           *
 *                                                                          *
 *==========================================================================*/

#include <getopt.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sepol/debug.h>
#include <sepol/policydb/policydb.h>
#include <sepol/policydb/link.h>
#include <sepol/policydb/services.h>
#include <sepol/policydb/avrule_block.h>

void *cmalloc(size_t s);
void set_attr(char *type, policydb_t *policy, int value);
int create_domain(char *d, policydb_t *policy);
int add_rule(char *s, char *t, char *c, char *p, policydb_t *policy);
int load_policy(char *filename, policydb_t *policydb, struct policy_file *pf);
int load_policy_into_kernel(policydb_t *policydb);
int set_live(char *, char *, char *, char *);
