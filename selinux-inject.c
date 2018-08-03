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
#include <sepol/policydb/expand.h>
#include <sepol/policydb/link.h>
#include <sepol/policydb/services.h>
#include <sepol/policydb/avrule_block.h>
#include <sepol/policydb/conditional.h>

#include <android/log.h>

#define  LOG_TAG    "selinux-inject"
#define  ALOG_ERROR(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)
#define  ALOG_INFO(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)

void *cmalloc(size_t s) {
	void *t = malloc(s);
	if (t == NULL) {
		ALOG_ERROR("Out of memory\n");
		exit(1);
	}
	return t;
}

void set_attr(char *type, policydb_t *policy, int value) {
	type_datum_t *attr = hashtab_search(policy->p_types.table, type);
	if (!attr) {
		ALOG_ERROR("%s not present in the policy\n", type);
		exit(1);
	}

	if (attr->flavor != TYPE_ATTRIB) {
		ALOG_ERROR("%s is not an attribute\n", type);
		exit(1);
	}

	if (ebitmap_set_bit(&attr->types, value - 1, 1)) {
		ALOG_ERROR("error setting attibute: %s\n", type);
		exit(1);
	}
}

int create_domain(char *d, policydb_t *policy) {
	symtab_datum_t *src = hashtab_search(policy->p_types.table, d);
	if (src)
		return src->value;

	type_datum_t *typdatum = (type_datum_t *) cmalloc(sizeof(type_datum_t));
	type_datum_init(typdatum);
	typdatum->primary = 1;
	typdatum->flavor = TYPE_TYPE;

	uint32_t value = 0;
	char *type = strdup(d);
	if (type == NULL) {
		exit(1);
	}
	int r = symtab_insert(policy, SYM_TYPES, type, typdatum, SCOPE_DECL, 1, &value);
	if (r) {
		ALOG_ERROR("Failed to insert type into symtab\n");
		exit(1);
	}
	typdatum->s.value = value;

	if (ebitmap_set_bit(&policy->global->branch_list->declared.scope[SYM_TYPES], value - 1, 1)) {
		exit(1);
	}

	policy->type_attr_map = realloc(policy->type_attr_map, sizeof(ebitmap_t)*policy->p_types.nprim);
	policy->attr_type_map = realloc(policy->attr_type_map, sizeof(ebitmap_t)*policy->p_types.nprim);
	ebitmap_init(&policy->type_attr_map[value-1]);
	ebitmap_init(&policy->attr_type_map[value-1]);
	ebitmap_set_bit(&policy->type_attr_map[value-1], value-1, 1);

	//Add the domain to all roles
	unsigned i;
	for (i = 0; i < policy->p_roles.nprim; ++i) {
		//Not sure all those three calls are needed
		ebitmap_set_bit(&policy->role_val_to_struct[i]->types.negset, value - 1, 0);
		ebitmap_set_bit(&policy->role_val_to_struct[i]->types.types, value - 1, 1);
		type_set_expand(&policy->role_val_to_struct[i]->types, &policy->role_val_to_struct[i]->cache, policy, 0);
	}

	src = hashtab_search(policy->p_types.table, d);
	if (!src) {
		ALOG_ERROR("creating %s failed\n",d);
		exit(1);
	}

	extern int policydb_index_decls(policydb_t * p);
	if (policydb_index_decls(policy)) {
		exit(1);
	}

	set_attr("domain", policy, value);
	return value;
}

int add_rule(char *s, char *t, char *c, char *p, policydb_t *policy) {
	type_datum_t *src, *tgt;
	class_datum_t *cls;
	perm_datum_t *perm;
	avtab_datum_t *av;
	avtab_key_t key;

	src = hashtab_search(policy->p_types.table, s);
	if (src == NULL) {
		ALOG_ERROR("source type %s does not exist\n", s);
		return 2;
	}
	tgt = hashtab_search(policy->p_types.table, t);
	if (tgt == NULL) {
		ALOG_ERROR("target type %s does not exist\n", t);
		return 2;
	}
	cls = hashtab_search(policy->p_classes.table, c);
	if (cls == NULL) {
		ALOG_ERROR("class %s does not exist\n", c);
		return 2;
	}

	uint32_t data = 0;

	char *p_copy = strdup(p);
	char *p_saveptr = NULL;
	char *p_token;
	if (p_copy == NULL) {
		ALOG_ERROR("memory allocation error\n");
		return 1;
	}
	p_token = strtok_r(p_copy, ",", &p_saveptr);
	while (p_token) {
		perm = hashtab_search(cls->permissions.table, p_token);
		if (perm == NULL) {
			if (cls->comdatum == NULL) {
				ALOG_ERROR("perm %s does not exist in class %s\n", p_token, c);
				return 2;
			}
			perm = hashtab_search(cls->comdatum->permissions.table, p_token);
			if (perm == NULL) {
				ALOG_ERROR("perm %s does not exist in class %s\n", p_token, c);
				return 2;
			}
		}
		data |= 1U << (perm->s.value - 1);
		p_token = strtok_r(NULL, ",", &p_saveptr);
	}
	free(p_copy);

	// See if there is already a rule
	key.source_type = src->s.value;
	key.target_type = tgt->s.value;
	key.target_class = cls->s.value;
	key.specified = AVTAB_ALLOWED;
	av = avtab_search(&policy->te_avtab, &key);

	if (av == NULL) {
		av = cmalloc(sizeof av);
		av->data = data;
		int ret = avtab_insert(&policy->te_avtab, &key, av);
		if (ret) {
			ALOG_ERROR("Error inserting into avtab\n");
			return 1;
		}
	}

	av->data |= data;

	return 0;
}
	

int load_policy(char *filename, policydb_t *policydb, struct policy_file *pf) {
	int fd;
	struct stat sb;
	void *map;
	int ret;

	fd = open(filename, O_RDONLY);
	if (fd < 0) {
		ALOG_ERROR("Can't open '%s':  %s\n",
			filename, strerror(errno));
		return 1;
	}
	if (fstat(fd, &sb) < 0) {
		ALOG_ERROR("Can't stat '%s':  %s\n",
			filename, strerror(errno));
		return 1;
	}
	map = mmap(NULL, sb.st_size, PROT_READ | PROT_WRITE, MAP_PRIVATE,
				fd, 0);
	if (map == MAP_FAILED) {
		ALOG_ERROR("Can't mmap '%s':  %s\n",
			filename, strerror(errno));
		close(fd);
		return 1;
	}

	policy_file_init(pf);
	pf->type = PF_USE_MEMORY;
	pf->data = map;
	pf->len = sb.st_size;
	if (policydb_init(policydb)) {
		ALOG_ERROR("policydb_init: Out of memory!\n");
		munmap(map, sb.st_size);
		close(fd);
		return 1;
	}
	ret = policydb_read(policydb, pf, 1);
	if (ret) {
		ALOG_ERROR("error(s) encountered while parsing configuration\n");
		munmap(map, sb.st_size);
		close(fd);
		return 1;
	}

	munmap(map, sb.st_size);
	close(fd);
	return 0;
}


int load_policy_into_kernel(policydb_t *policydb) {
	char *filename = "/sys/fs/selinux/load";
	int fd, ret;
	void *data = NULL;
	size_t len;

	policydb_to_image(NULL, policydb, &data, &len);

	// based on libselinux security_load_policy()
	fd = open(filename, O_RDWR);
	if (fd < 0) {
		ALOG_ERROR("Can't open '%s':  %s\n",
		        filename, strerror(errno));
		return 1;
	}
	ret = write(fd, data, len);
	close(fd);
	if (ret < 0) {
		ALOG_ERROR("Could not write policy to %s\n",
		        filename);
		return 1;
	}
	return 0;
}

int set_live(char *source, char *target, char *class, char *perms)
{	
	policydb_t policydb;
	struct policy_file pf;
	sidtab_t sidtab;
	int typeval;
	type_datum_t *type;

	sepol_set_policydb(&policydb);
	sepol_set_sidtab(&sidtab);

	if (load_policy("/sys/fs/selinux/policy", &policydb, &pf)) {
		ALOG_ERROR("Could not load policy\n");
		return 1;
	}

	if (policydb_load_isids(&policydb, &sidtab))
		return 1;

	type = hashtab_search(policydb.p_types.table, source);
	if (type == NULL) {
		ALOG_ERROR("type %s does not exist, creating\n", source);
		typeval = create_domain(source, &policydb);
	} else {
		typeval = type->s.value;
	}

	int ret_add_rule;
	if (ret_add_rule = add_rule(source, target, class, perms, &policydb)) {
		ALOG_ERROR("Could not add rule\n");
		return ret_add_rule;
	}

	if (load_policy_into_kernel(&policydb)) {
		ALOG_ERROR("Could not load new policy into kernel\n");
		return 1;
	}

	policydb_destroy(&policydb);

	ALOG_INFO("Successfully modified device's SEPolicy\n");
	return 0;
}
