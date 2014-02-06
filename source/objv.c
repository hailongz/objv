//
//  objv.c
//  objv
//
//  Created by zhang hailong on 14-1-29.
//  Copyright (c) 2014年 hailong.org. All rights reserved.
//

#define READONLY

#include "objv_os.h"
#include "objv.h"
#include "objv_private.h"
#include "objv_hash_map.h"
#include "objv_log.h"

OBJV_KEY_IMP(alloc)
OBJV_KEY_IMP(dealloc)
OBJV_KEY_IMP(retainCount)
OBJV_KEY_IMP(equal)
OBJV_KEY_IMP(hashCode)
OBJV_KEY_IMP(Object)


objv_boolean_t objv_key_equal(objv_key_t * key1,objv_key_t * key2){
    
    if(key1 == key2){
        return objv_true;
    }
    
    if(key1->name == key2->name){
        return objv_true;
    }
    
    if(key1->type != key2->type){
        return strcmp(key1->name, key2->name) == 0 ? objv_true : objv_false;
    }
    
    return objv_false;
}

static void objv_object_methods_dealloc(objv_class_t * clazz, objv_object_t * obj){
    
    //objv_log("\n%s(0x%x) dealloc\n", obj->isa->name->name,(unsigned long) obj);
    
    if(clazz->superClass){
        objv_object_dealloc(clazz->superClass,obj);
    }
}

static long objv_object_method_hashCode (objv_class_t * clazz, objv_object_t * object){
    return (long) object;
}

static objv_boolean_t objv_object_method_equal(objv_class_t * clazz, objv_object_t * object,objv_object_t * value){
    return object == value;
}


static int objv_object_methods_retainCount(objv_object_t * obj){
    return obj->retainCount;
}

static objv_method_t objv_object_methods[] = {
    {OBJV_KEY(dealloc),"v()",(objv_method_impl_t)objv_object_methods_dealloc}
    ,{OBJV_KEY(hashCode),"l()",(objv_method_impl_t)objv_object_method_hashCode}
    ,{OBJV_KEY(equal),"b()",(objv_method_impl_t)objv_object_method_equal}
    ,{OBJV_KEY(retainCount),"i()",(objv_method_impl_t)objv_object_methods_retainCount}
};


static void objv_object_class_initialize(objv_class_t * clazz){
    
}

objv_class_t objv_object_class = {OBJV_KEY(Object),NULL
    ,objv_object_methods,sizeof(objv_object_methods) / sizeof(objv_method_t)
    ,NULL,0
    ,sizeof(objv_object_t)
    ,objv_object_class_initialize,0,0};

static objv_hash_map_t * _objv_classs = NULL;
static objv_mutex_t _objv_classs_mutex;


objv_class_t * objv_class(objv_key_t * className){
    
    if(className && _objv_classs){
        
        objv_mutex_lock(& _objv_classs_mutex);
        
        objv_class_t * clazz = (objv_class_t *) objv_hash_map_get(_objv_classs, (void *) className->name);
        
        objv_mutex_unlock(& _objv_classs_mutex);
     
        return clazz;
    }
    
    return NULL;
}

static void objv_class_reg(objv_class_t * clazz){
    
    if(clazz){
        
        if(_objv_classs == NULL){
            
            objv_mutex_init(& _objv_classs_mutex);
            
            _objv_classs = objv_hash_map_alloc(32, objv_hash_map_hash_code_string, objv_map_compare_string);
        }
        
        objv_mutex_lock(& _objv_classs_mutex);
        
        if(! objv_hash_map_get(_objv_classs, (void *) clazz->name)){
            objv_hash_map_put(_objv_classs, (void *) clazz->name, clazz);
        }
        
        objv_mutex_unlock(& _objv_classs_mutex);
        
    }
}

void objv_class_initialize(objv_class_t * clazz){
   
    if(clazz && ! clazz->initialized){
        
        if(clazz->superClass){
            objv_class_initialize(clazz->superClass);
            clazz->offset = clazz->superClass->size;
        }
        
        if(clazz->name->type == objv_key_type_static){
            objv_class_reg(clazz);
        }
    
        if(clazz->initialize){
            (* clazz->initialize)(clazz);
        }
        
        clazz->initialized = objv_true;
    }
}

objv_method_t * objv_class_getMethod(objv_class_t * clazz,objv_key_t * name){
    
    if(clazz && name){
        
        objv_method_t * p = clazz->methods;
        int c = clazz->methodCount;
        
        while (c >0 ) {
            
            if(name == p->name
               || (p->name->type == name->type && p->name->name == name->name)
               || (p->name->type != name->type && p->name->name == name->name)){
                
                return p;
            }
            
            c --;
            p ++;
        }
    }
    
    return NULL;
}

objv_property_t * objv_class_getProperty(objv_class_t * clazz,objv_key_t * name){
    
    if(clazz && name){
        
        objv_property_t * p = clazz->propertys;
        int c = clazz->propertyCount;
        
        while (c >0 ) {
            
            if(objv_key_equal(name ,p->name ) ){
                
                return p;
            }
            
            c --;
            p ++;
        }
    }
    
    return NULL;
}

objv_boolean_t objv_class_isKindOfClass(objv_class_t * clazz,objv_class_t * ofClass){
    
    objv_class_t * c = clazz;
    
    while (c) {
        if(c == ofClass){
            return objv_true;
        }
        
        c = c->superClass;
    }
    
    return objv_false;
}

objv_object_t * objv_object_allocv(objv_zone_t * zone,objv_class_t * clazz,va_list ap){
    
    
    if(zone == NULL){
        zone = objv_zone_default();
    }
    
    objv_class_initialize(clazz);
    
    if(clazz && clazz->size > 0){
        
        objv_object_t * obj = (objv_object_t *) objv_zone_malloc(zone, clazz->size);
        
        objv_zone_memzero(zone, obj, clazz->size);
        
        obj->isa = clazz;
        obj->zone = zone;
        obj->retainCount = 1;
        objv_mutex_init(& obj->mutex);
        
        {
            objv_class_t * c = clazz;
            
            objv_method_t * method = NULL;
            
            while(c && (method = objv_class_getMethod(c, OBJV_KEY(alloc))) == NULL){
                c = c->superClass;
            }
            
            if(method){
                obj = (* (objv_object_method_alloc_t) method->impl)(c,obj,ap);
            }
        }
        
        return obj;
        
    }
    
    return NULL;
    
}

objv_object_t * objv_object_alloc(objv_zone_t * zone,objv_class_t * clazz,...){
    
    va_list ap;
    objv_object_t * object = NULL;
    
    va_start(ap, clazz);
    
    object = objv_object_allocv(zone,clazz,ap);
    
    va_end(ap);
    
    return object;
}

objv_object_t * objv_object_retain(objv_object_t * object){
    
    if(object){
        
        objv_mutex_lock(& object->mutex);
        
        assert(object->retainCount > 0);
        
        object->retainCount ++;
        
        objv_mutex_unlock(& object->mutex);
    }
    
    return object;
}


void objv_object_release(objv_object_t * object){
    
    if(object){
        
        objv_mutex_lock(& object->mutex);
        
        assert(object->retainCount > 0);
        
        object->retainCount -- ;
        
        int dodealloc = object->retainCount == 0;
        
        objv_mutex_unlock(& object->mutex);

        if( dodealloc ){
            
            objv_object_dealloc(object->isa,object);
            
            objv_mutex_destroy(& object->mutex);
            
            objv_zone_free(object->zone, object);
            
        }
    }
    
}

void objv_object_lock(objv_object_t * object){

    if(object){
        
        objv_mutex_lock(& object->mutex);
        
    }
    
}

void objv_object_unlock(objv_object_t * object){
    
    if(object){
        
        objv_mutex_unlock(& object->mutex);
        
    }
}

objv_boolean_t objv_object_isKindOfClass(objv_object_t * object,objv_class_t * ofClass){
    if(object && ofClass){
        return objv_class_isKindOfClass(object->isa, ofClass);
    }
    return objv_false;
}


void objv_object_dealloc(objv_class_t * clazz, objv_object_t * object){
    
    if(clazz && object){
        
        objv_class_t * c = clazz;
        
        objv_method_t * method = NULL;
        
        while(c && (method = objv_class_getMethod(c, OBJV_KEY(dealloc))) == NULL){
            
            c = c->superClass;
        }
        
        if(method){
            (* (objv_object_method_dealloc_t) method->impl)(c,object);
        }
    }
    
}

long objv_object_hashCode(objv_class_t * clazz,objv_object_t * object){
    
    if(clazz && object){
        
        objv_class_t * c = clazz;
        
        objv_method_t * method = NULL;
        
        while(c && (method = objv_class_getMethod(c, OBJV_KEY(hashCode))) == NULL){
            
            c = c->superClass;
        }
        
        if(method){
            return (* (objv_object_method_hashCode_t) method->impl)(c,object);
        }
        
    }
    
    return 0;
}


objv_boolean_t objv_object_equal(objv_class_t * clazz,objv_object_t * object
                                 ,objv_object_t * value){
    
    if(clazz && object){
        
        objv_class_t * c = clazz;
        
        objv_method_t * method = NULL;
        
        while(c && (method = objv_class_getMethod(c, OBJV_KEY(equal))) == NULL){
            
            c = c->superClass;
        }
        
        if(method){
            return (* (objv_object_method_equal_t) method->impl)(c,object,value);
        }
        
    }
    
    return object == value;
}


