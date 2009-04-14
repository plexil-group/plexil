#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <assert.h>

#include <sys/stat.h>
#include <sys/types.h>

#include "lcmgen.h"
#include "sprintfalloc.h"
#include "getopt.h"

#define INDENT(n) (4*(n))

#define emit_start(n, ...) do { fprintf(f, "%*s", INDENT(n), ""); fprintf(f, __VA_ARGS__); } while (0)
#define emit_continue(...) do { fprintf(f, __VA_ARGS__); } while (0)
#define emit_end(...) do { fprintf(f, __VA_ARGS__); fprintf(f, "\n"); } while (0)
#define emit(n, ...) do { fprintf(f, "%*s", INDENT(n), ""); fprintf(f, __VA_ARGS__); fprintf(f, "\n"); } while (0)

static char *dots_to_slashes(const char *s)
{
    char *p = strdup(s);

    for (char *t=p; *t!=0; t++)
        if (*t == '.')
            *t = '/';

    return p;
}

static void make_dirs_for_file(const char *path)
{
    int len = strlen(path);
    for (int i = 0; i < len; i++) {
        if (path[i]=='/') {
            char *dirpath = malloc(i+1);
            strncpy(dirpath, path, i);
            dirpath[i]=0;

            mkdir(dirpath, 0755);
            free(dirpath);

            i++; // skip the '/'
        }
    }
}

void setup_java_options(getopt_t *gopt)
{
    getopt_add_string(gopt, 0,   "jpath",     "",         "Java file destination directory");
    getopt_add_bool(gopt, 0,    "jmkdir",     1,         "Make java source directories automatically");
    getopt_add_string(gopt, 0,   "jdecl",      "implements lcm.lcm.LCMEncodable", "String added to class declarations");
    getopt_add_string(gopt, 0,   "jdefaultpkg", "lcmtypes", "Default Java package if LCM type has no package");
}

typedef struct
{
    char *storage;
    char *decode;
    char *encode;
} primitive_info_t;

static primitive_info_t *prim(char *storage, char *decode, char *encode)
{
    primitive_info_t *p = (primitive_info_t*) calloc(sizeof(primitive_info_t), 1);
    p->storage = storage;
    p->decode = decode;
    p->encode = encode;

    return p;
}

static int jdefaultpkg_warned = 0;

const char *make_fqn(lcmgen_t *lcm, const char *type_name)
{
    if (strchr(type_name, '.')!=NULL)
        return type_name;

    if (!jdefaultpkg_warned) {
        printf("Notice: enclosing LCM types without package into java namespace '%s'.\n", getopt_get_string(lcm->gopt, "jdefaultpkg"));
        jdefaultpkg_warned = 1;
    }

    return sprintfalloc("%s.%s", getopt_get_string(lcm->gopt, "jdefaultpkg"), type_name);
}

/** # -> replace1
    @ -> replace2
**/
static void freplace(FILE *f, const char *haystack, const char *replace1)
{
    int len = strlen(haystack);
    
    for (int pos = 0; pos < len; pos++)
    {
        if (haystack[pos]=='#')
            fprintf(f, "%s", replace1);
        else
            fprintf(f, "%c", haystack[pos]);
    }
}

static void make_accessor(lcm_member_t *lm, const char *obj, char *s)
{
    int ndim = g_ptr_array_size(lm->dimensions);
    int pos = 0;
    s[0] = 0;

    pos += sprintf(s, "%s%s%s", obj, obj[0]==0 ? "" : ".", lm->membername);
    for (int d = 0 ; d < ndim; d++) 
        pos += sprintf(&s[pos],"[%c]", 'a'+d);
}

int emit_java(lcmgen_t *lcm)
{
    GHashTable *type_table = g_hash_table_new(g_str_hash, g_str_equal);

    g_hash_table_insert(type_table, "byte",   prim("byte",  
                                             "# = ins.readByte();",  
                                             "outs.writeByte(#);"));
    g_hash_table_insert(type_table, "int8_t",   prim("byte",  
                                               "# = ins.readByte();",  
                                               "outs.writeByte(#);"));
    g_hash_table_insert(type_table, "int16_t",  prim("short", 
                                               "# = ins.readShort();", 
                                               "outs.writeShort(#);"));
    g_hash_table_insert(type_table, "int32_t",  prim("int",   
                                               "# = ins.readInt();",   
                                               "outs.writeInt(#);"));
    g_hash_table_insert(type_table, "int64_t",  prim("long",  
                                               "# = ins.readLong();",  
                                               "outs.writeLong(#);"));
    g_hash_table_insert(type_table, "string",   prim("String",
                                               "__strbuf = new byte[ins.readInt()-1]; ins.readFully(__strbuf); ins.readByte(); # = new String(__strbuf, \"UTF-8\");",
                                               "__strbuf = #.getBytes(\"UTF-8\"); outs.writeInt(__strbuf.length+1); outs.write(__strbuf, 0, __strbuf.length); outs.writeByte(0);"));
    g_hash_table_insert(type_table, "boolean",  prim("boolean",
                                               "# = ins.readByte()!=0;",
                                               "outs.writeByte( # ? 1 : 0);"));
    g_hash_table_insert(type_table, "float",    prim("float",
                                               "# = ins.readFloat();",
                                               "outs.writeFloat(#);"));
    g_hash_table_insert(type_table, "double",   prim("double",
                                               "# = ins.readDouble();",
                                               "outs.writeDouble(#);"));
    
    //////////////////////////////////////////////////////////////
    // ENUMS
    for (unsigned int en = 0; en < g_ptr_array_size(lcm->enums); en++) {
        lcm_enum_t *le = g_ptr_array_index(lcm->enums, en);
        
        const char *classname = make_fqn(lcm, le->enumname->typename);
        char *path = sprintfalloc("%s%s%s.java", 
                                  getopt_get_string(lcm->gopt, "jpath"),
                                  strlen(getopt_get_string(lcm->gopt, "jpath")) > 0 ? "/" : "",
                                  dots_to_slashes(classname));

        if (!lcm_needs_generation(lcm, le->lcmfile, path))
            continue;

        if (getopt_get_bool(lcm->gopt, "jmkdir"))
            make_dirs_for_file(path);

        FILE *f = fopen(path, "w");
        if (f==NULL)
            return -1;

        if (strlen(le->enumname->package) > 0)
            emit(0, "package %s;", le->enumname->package);
        else
            emit(0, "package %s;", getopt_get_string(lcm->gopt, "jdefaultpkg"));

        emit(0, " ");
        emit(0, "import java.io.*;");
        emit(0, "import java.util.*;");
        emit(0, " ");

        emit(0, "public class %s %s", le->enumname->shortname, getopt_get_string(lcm->gopt, "jdecl"));

        emit(0, "{");
        emit(1, "public int value;");
        emit(0, " ");

        for (unsigned int v = 0; v < g_ptr_array_size(le->values); v++) {
            lcm_enum_value_t *lev = g_ptr_array_index(le->values, v);
            emit(1, "public static final int %-16s = %i;", 
                    lev->valuename, lev->value);
        }
        emit(0," ");

        emit(1,"public %s(int value) { this.value = value; }", 
                le->enumname->shortname);
        emit(0," ");

        emit(1,"public int getValue() { return value; }");
        emit(0," ");

        emit(1,"public void _encodeRecursive(DataOutputStream outs) throws IOException");
        emit(1,"{");
        emit(2,"outs.writeInt(this.value);");
        emit(1,"}");
        emit(0," ");

        emit(1,"public void encode(DataOutputStream outs) throws IOException");
        emit(1,"{");
        emit(2,"outs.writeLong(LCM_FINGERPRINT);");
        emit(2,"_encodeRecursive(outs);");
        emit(1,"}");
        emit(0," ");

        emit(1,"public static %s _decodeRecursiveFactory(DataInputStream ins) throws IOException", make_fqn(lcm, le->enumname->typename));
        emit(1,"{");
        emit(2,"%s o = new %s(0);", make_fqn(lcm, le->enumname->typename), make_fqn(lcm, le->enumname->typename));
        emit(2,"o._decodeRecursive(ins);");
        emit(2,"return o;");
        emit(1,"}");
        emit(0," ");

        emit(1,"public void _decodeRecursive(DataInputStream ins) throws IOException");
        emit(1,"{");
        emit(2,"this.value = ins.readInt();");
        emit(1,"}");
        emit(0," ");

        emit(1,"public %s(DataInputStream ins) throws IOException", le->enumname->shortname);
        emit(1,"{");
        emit(2,"long hash = ins.readLong();");
        emit(2,"if (hash != LCM_FINGERPRINT)");
        emit(3,     "throw new IOException(\"LCM Decode error: bad fingerprint\");");
        emit(2,"_decodeRecursive(ins);");
        emit(1,"}");
        emit(0," ");

        emit(1,"public %s copy()", classname);
        emit(1,"{");
        emit(2,"return new %s(this.value);", classname);
        emit(1,"}");
        emit(0," ");

        emit(1,"public static final long _hashRecursive(ArrayList<Class> clss)");
        emit(1,"{");
        emit(2,"return LCM_FINGERPRINT;");
        emit(1,"}");
        emit(0," ");
        emit(1, "public static final long LCM_FINGERPRINT = 0x%016"PRIx64"L;", le->hash);
        emit(0, "}");
        fclose(f);
    }
    
    for (unsigned int st = 0; st < g_ptr_array_size(lcm->structs); st++) {
        lcm_struct_t *lr = g_ptr_array_index(lcm->structs, st);

        const char *classname = make_fqn(lcm, lr->structname->typename);
        char *path = sprintfalloc("%s%s%s.java", 
                                  getopt_get_string(lcm->gopt, "jpath"), 
                                  strlen(getopt_get_string(lcm->gopt, "jpath")) > 0 ? "/" : "",
                                  dots_to_slashes(classname));

        if (!lcm_needs_generation(lcm, lr->lcmfile, path))
            continue;

        if (getopt_get_bool(lcm->gopt, "jmkdir"))
            make_dirs_for_file(path);

        FILE *f = fopen(path, "w");
        if (f==NULL)
            return -1;

        if (strlen(lr->structname->package) > 0)
            emit(0, "package %s;", lr->structname->package);
        else
            emit(0, "package %s;", getopt_get_string(lcm->gopt, "jdefaultpkg"));
        
        emit(0, " ");
        emit(0, "import java.io.*;");
        emit(0, "import java.util.*;");
        emit(0, " ");
        emit(0, "public class %s %s", lr->structname->shortname, getopt_get_string(lcm->gopt, "jdecl"));
        emit(0, "{");

        for (unsigned int member = 0; member < g_ptr_array_size(lr->members); member++) {
            lcm_member_t *lm = g_ptr_array_index(lr->members, member);
            primitive_info_t *pinfo = (primitive_info_t*) g_hash_table_lookup(type_table, lm->type->typename);

            emit_start(1, "public ");
            
            if (pinfo==NULL)  {
                emit_continue(make_fqn(lcm, lm->type->typename));
            } else {
                emit_continue("%s", pinfo->storage);
            }

            emit_continue(" %s", lm->membername);
            for (unsigned int i = 0; i < g_ptr_array_size(lm->dimensions); i++)
                emit_continue("[]");
            emit_end(";");
        }
        emit(0," ");

        // public constructor
        emit(1,"public %s()", lr->structname->shortname);
        emit(1,"{");

        // pre-allocate any fixed-size arrays.
        for (unsigned int member = 0; member < g_ptr_array_size(lr->members); member++) {
            lcm_member_t *lm = g_ptr_array_index(lr->members, member);
            primitive_info_t *pinfo = (primitive_info_t*) g_hash_table_lookup(type_table, lm->type->typename);

            if (g_ptr_array_size(lm->dimensions)==0 || !lcm_is_constant_size_array(lm))
                continue;

            emit_start(2, "%s = new ", lm->membername);
            if (pinfo != NULL)
                emit_continue(pinfo->storage);
            else 
                emit_continue(make_fqn(lcm, lm->type->typename));
      
            for (unsigned int i = 0; i < g_ptr_array_size(lm->dimensions); i++) {
                lcm_dimension_t *dim = (lcm_dimension_t*) g_ptr_array_index(lm->dimensions, i);
                emit_continue("[%s]", dim->size);
            }
            emit_end(";");
        }
        emit(1,"}");
        emit(0," ");

        emit(1, "public static final long LCM_FINGERPRINT;");
        emit(1, "public static final long LCM_FINGERPRINT_BASE = 0x%016"PRIx64"L;", lr->hash);
        emit(0," ");

        //////////////////////////////////////////////////////////////
        // CONSTANTS
        for (unsigned int cn = 0; cn < g_ptr_array_size(lr->constants); cn++) {
            lcm_constant_t *lc = g_ptr_array_index(lr->constants, cn);
            assert(lcm_is_legal_const_type(lc->typename));

            if (!strcmp(lc->typename, "int8_t") ||
                !strcmp(lc->typename, "int16_t") ||
                !strcmp(lc->typename, "int32_t")) {
                emit(1, "public static final int %s = %s;", lc->membername, lc->val_str);
            } else if (!strcmp(lc->typename, "int64_t")) {
                emit(1, "public static final long %s = %sL;", lc->membername, lc->val_str);
            } else if (!strcmp(lc->typename, "float")) {
                emit(1, "public static final float %s = %s;", lc->membername, lc->val_str);
            } else if (!strcmp(lc->typename, "double")) {
                emit(1, "public static final double %s = %s;", lc->membername, lc->val_str);
            } else {
                assert(0);
            }
        }
        if (g_ptr_array_size(lr->constants) > 0)
            emit(0, "");

        ///////////////// encode //////////////////
        emit(1, "static {");
        emit(2, "LCM_FINGERPRINT = _hashRecursive(new ArrayList<Class>());");
        emit(1, "}");
        emit(0, " ");

        emit(1, "public static long _hashRecursive(ArrayList<Class> classes)");
        emit(1, "{");
        emit(2, "if (classes.contains(%s.class))", make_fqn(lcm, lr->structname->typename));
        emit(3,     "return 0L;");
        emit(0, " ");
        emit(2, "classes.add(%s.class);", make_fqn(lcm, lr->structname->typename));

        emit(2, "long hash = LCM_FINGERPRINT_BASE");
        for (unsigned int member = 0; member < g_ptr_array_size(lr->members); member++) {
            lcm_member_t *lm = g_ptr_array_index(lr->members, member);
            primitive_info_t *pinfo = (primitive_info_t*) g_hash_table_lookup(type_table, lm->type->typename);

            if (pinfo)
                continue;

            emit(3, " + %s._hashRecursive(classes)", make_fqn(lcm, lm->type->typename));
        }
        emit(3,";");

        emit(2, "classes.remove(classes.size() - 1);");
        emit(2, "return (hash<<1) + ((hash>>63)&1);");

        emit(1, "}");
        emit(0, " ");

        ///////////////// encode //////////////////

        emit(1,"public void encode(DataOutputStream outs) throws IOException");
        emit(1,"{");
        emit(2,"outs.writeLong(LCM_FINGERPRINT);");
        emit(2,"_encodeRecursive(outs);");
        emit(1,"}");
        emit(0," ");

        emit(1,"public void _encodeRecursive(DataOutputStream outs) throws IOException");
        emit(1,"{");
        emit(2, "byte[] __strbuf = null;");
        char accessor[1024];

        for (unsigned int member = 0; member < g_ptr_array_size(lr->members); member++) {
            lcm_member_t *lm = g_ptr_array_index(lr->members, member);
            primitive_info_t *pinfo = (primitive_info_t*) g_hash_table_lookup(type_table, lm->type->typename);
            make_accessor(lm, "this", accessor);

            for (unsigned int i = 0; i < g_ptr_array_size(lm->dimensions); i++) {
                lcm_dimension_t *dim = (lcm_dimension_t*) g_ptr_array_index(lm->dimensions, i);
                emit(2+i, "for (int %c = 0; %c < %s; %c++) {", 'a'+i, 'a'+i, dim->size, 'a'+i);
            }
            
            emit_start(2 + g_ptr_array_size(lm->dimensions),"");
            if (pinfo != NULL)
                freplace(f, pinfo->encode, accessor);
            else
                freplace(f, "#._encodeRecursive(outs);", accessor);
            emit_end(" ");                

            for (unsigned int i = 0; i < g_ptr_array_size(lm->dimensions); i++) {
                emit(2 + g_ptr_array_size(lm->dimensions) - 1, "}");
            }
            emit(0," ");
        }
        emit(1,"}");
        emit(0," ");

        ///////////////// decode //////////////////

        // decoding constructors
        emit(1, "public %s(byte[] data) throws IOException", lr->structname->shortname);
        emit(1, "{");
        emit(2, "this(new DataInputStream(new ByteArrayInputStream(data)));");
        emit(1, "}");
        emit(0, " ");

        emit(1,"public %s(DataInputStream ins) throws IOException", lr->structname->shortname);
        emit(1,"{");
        emit(2,"if (ins.readLong() != LCM_FINGERPRINT)");
        emit(3,     "throw new IOException(\"LCM Decode error: bad fingerprint\");");
        emit(0," ");
        emit(2,"_decodeRecursive(ins);");
        emit(1,"}");
        emit(0," ");

        emit(1,"public static %s _decodeRecursiveFactory(DataInputStream ins) throws IOException", make_fqn(lcm, lr->structname->typename));
        emit(1,"{");
        emit(2,"%s o = new %s();", make_fqn(lcm, lr->structname->typename), make_fqn(lcm, lr->structname->typename));
        emit(2,"o._decodeRecursive(ins);");
        emit(2,"return o;");
        emit(1,"}");
        emit(0," ");

        emit(1,"public void _decodeRecursive(DataInputStream ins) throws IOException");
        emit(1,"{");        
        emit(2,     "byte[] __strbuf = null;");
        for (unsigned int member = 0; member < g_ptr_array_size(lr->members); member++) {
            lcm_member_t *lm = g_ptr_array_index(lr->members, member);
            primitive_info_t *pinfo = (primitive_info_t*) g_hash_table_lookup(type_table, lm->type->typename);

            make_accessor(lm, "this", accessor);

            // allocate an array if necessary
            if (g_ptr_array_size(lm->dimensions) > 0) {

                emit_start(2, "this.%s = new ", lm->membername);

                if (pinfo != NULL)
                    emit_continue(pinfo->storage);
                else
                    emit_continue(make_fqn(lcm, lm->type->typename));

                for (unsigned int i = 0; i < g_ptr_array_size(lm->dimensions); i++) {
                    lcm_dimension_t *dim = (lcm_dimension_t*) g_ptr_array_index(lm->dimensions, i);
                    emit_continue("[(int) %s]", dim->size);
                }
                emit_end(";");
            }

            for (unsigned int i = 0; i < g_ptr_array_size(lm->dimensions); i++) {
                lcm_dimension_t *dim = (lcm_dimension_t*) g_ptr_array_index(lm->dimensions, i);
                emit(2+i, "for (int %c = 0; %c < %s; %c++) {", 'a'+i, 'a'+i, dim->size, 'a'+i);
            }
            
            emit_start(2 + g_ptr_array_size(lm->dimensions),"");
            if (pinfo != NULL) 
                freplace(f, pinfo->decode, accessor);
            else {
                emit_continue("%s = %s._decodeRecursiveFactory(ins);", accessor, make_fqn(lcm, lm->type->typename));
            }
            emit_end("");

            for (unsigned int i = 0; i < g_ptr_array_size(lm->dimensions); i++) {
                emit(2 + g_ptr_array_size(lm->dimensions) - 1, "}");
            }

            emit(0," ");
        }

        emit(1,"}");
        emit(0," ");


        ///////////////// copy //////////////////

        emit(1,"public %s copy()", classname);
        emit(1,"{");
        emit(2,"%s outobj = new %s();", classname, classname);

        for (unsigned int member = 0; member < g_ptr_array_size(lr->members); member++) {
            lcm_member_t *lm = g_ptr_array_index(lr->members, member);
            primitive_info_t *pinfo = (primitive_info_t*) g_hash_table_lookup(type_table, lm->type->typename);
            make_accessor(lm, "", accessor);

            // allocate an array if necessary
            if (g_ptr_array_size(lm->dimensions) > 0) {

                emit_start(2, "outobj.%s = new ", lm->membername);

                if (pinfo != NULL)
                    emit_continue(pinfo->storage);
                else
                    emit_continue(make_fqn(lcm, lm->type->typename));

                for (unsigned int i = 0; i < g_ptr_array_size(lm->dimensions); i++) {
                    lcm_dimension_t *dim = (lcm_dimension_t*) g_ptr_array_index(lm->dimensions, i);
                    emit_continue("[(int) %s]", dim->size);
                }
                emit_end(";");
            }


            for (unsigned int i = 0; i < g_ptr_array_size(lm->dimensions); i++) {
                lcm_dimension_t *dim = (lcm_dimension_t*) g_ptr_array_index(lm->dimensions, i);
                emit(2+i, "for (int %c = 0; %c < %s; %c++) {", 'a'+i, 'a'+i, dim->size, 'a'+i);
            }
            
            if (pinfo != NULL) {

                emit_start(2+g_ptr_array_size(lm->dimensions), "outobj.%s", lm->membername);
                for (unsigned int i = 0; i < g_ptr_array_size(lm->dimensions); i++) {
                    emit_continue("[%c]", 'a'+i);
                }
                emit_continue(" = this.%s", lm->membername);
                
                for (unsigned int i = 0; i < g_ptr_array_size(lm->dimensions); i++) {
                    emit_continue("[%c]", 'a'+i);
                }

                emit_end(";");
                
            } else {
                emit_continue("outobj.%s = this.%s.copy();", accessor, accessor);
            }

            for (unsigned int i = 0; i < g_ptr_array_size(lm->dimensions); i++) {
                emit(2 + g_ptr_array_size(lm->dimensions) - 1, "}");
            }

            emit(0," ");
        }

        emit(2,"return outobj;");
        emit(1,"}");
        emit(0," ");

        ////////
        emit(0, "}\n");
        fclose(f);
    }
 
/* XXX unfinished
   
    hashtable_iterator_t *hit = hashtable_iterator_create(type_table);
    hashtable_entry_t *entry;
    while ((entry = hashtable_iterator_next(hit)) != NULL) {
        free((char*) entry->value);
    }
    hashtable_iterator_destroy(hit);

    hashtable_destroy(type_table);
*/
    return 0;
}
