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
#include "objv_iterator.h"
#include "objv_autorelease.h"
#include "objv_value.h"
#include "objv_hash_map.h"

OBJV_KEY_IMP(init)
OBJV_KEY_IMP(dealloc)
OBJV_KEY_IMP(retainCount)
OBJV_KEY_IMP(equal)
OBJV_KEY_IMP(hashCode)
OBJV_KEY_IMP(copy)
OBJV_KEY_IMP(Object)

objv_boolean_t objv_key_equal(objv_key_t * key1,objv_key_t * key2){
    
    if(key1 == key2){
        return objv_true;
    }
    
    return strcmp(key1, key2) == 0 ? objv_true : objv_false;
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


static int objv_object_methods_retainCount(objv_class_t * clazz,objv_object_t * obj){
    return obj->retainCount;
}

static objv_iterator_t * objv_object_methods_keyIterator(objv_class_t * clazz,objv_object_t * obj){
    return (objv_iterator_t * ) objv_object_autorelease( (objv_object_t *) objv_object_iterator_alloc(obj));
}

static objv_object_t * objv_object_methods_objectForKey(objv_class_t * clazz,objv_object_t * obj,objv_object_t * key){
    
    
    objv_class_t * c = obj->isa;
    objv_property_t * prop;
    unsigned int propCount;
    
    objv_string_t * skey = objv_object_stringValue(key, NULL);
    
    if(skey){

        while(c){
            
            prop = c->propertys;
            propCount = c->propertyCount;
            
            while(prop && propCount >0){
                
                if(prop->name == skey->UTF8String || strcmp(prop->name, skey->UTF8String)){
                    
                    return objv_property_objectValue(c, obj, prop, NULL);
                    
                }
                
                prop ++;
                propCount --;
            }
            
            c = c->superClass;
        }
    }
    
    return NULL;
}

static void objv_object_methods_setObjectForKey(objv_class_t * clazz,objv_object_t * obj
                                                ,objv_object_t * key,objv_object_t * value){
    objv_class_t * c = obj->isa;
    objv_property_t * prop;
    unsigned int propCount;
    
    objv_string_t * skey = objv_object_stringValue(key, NULL);
    
    if(skey){
        
        while(c){
            
            prop = c->propertys;
            propCount = c->propertyCount;
            
            while(prop && propCount >0){
                
                if(prop->name == skey->UTF8String || strcmp(prop->name, skey->UTF8String)){
                    
                    objv_property_setObjectValue(c, obj, prop, value);

                    return;
                }
                
                prop ++;
                propCount --;
            }
            
            c = c->superClass;
        }
    }
    
}

OBJV_CLASS_METHOD_IMP_BEGIN(Object)

OBJV_CLASS_METHOD_IMP(dealloc, "v()", objv_object_methods_dealloc)
OBJV_CLASS_METHOD_IMP(hashCode,"l()",objv_object_method_hashCode)
OBJV_CLASS_METHOD_IMP(equal,"b()",objv_object_method_equal)
OBJV_CLASS_METHOD_IMP(retainCount,"i()",objv_object_methods_retainCount)
OBJV_CLASS_METHOD_IMP(keyIterator,"@()",objv_object_methods_keyIterator)
OBJV_CLASS_METHOD_IMP(objectForKey,"@(@)",objv_object_methods_objectForKey)
OBJV_CLASS_METHOD_IMP(setObjectForKey,"v(@,@)",objv_object_methods_setObjectForKey)

OBJV_CLASS_METHOD_IMP_END(Object)

static objv_hash_map_t * _objv_classs = NULL;
static objv_mutex_t _objv_classs_mutex;


void objv_class_reg(objv_class_t * clazz){
    
    if(clazz){
        
        if(_objv_classs == NULL){
            
            objv_mutex_init(& _objv_classs_mutex);
            
            _objv_classs = objv_hash_map_alloc(32, objv_hash_map_hash_code_key, objv_map_compare_key);
        }
        
        objv_mutex_lock(& _objv_classs_mutex);
        
        if(! objv_hash_map_get(_objv_classs, (void *) clazz->name)){
            objv_hash_map_put(_objv_classs, (void *) clazz->name, clazz);
        }
        
        objv_mutex_unlock(& _objv_classs_mutex);
        
    }
}


static void objv_object_class_initialize(objv_class_t * clazz){
    objv_class_reg(clazz);
}

objv_class_t objv_Object_class = {OBJV_KEY(Object),NULL
    ,objv_Object_methods,sizeof(objv_Object_methods) / sizeof(objv_method_t)
    ,NULL,0
    ,sizeof(objv_object_t)
    ,objv_object_class_initialize,0};


objv_class_t * objv_class(objv_key_t * className){
    
    if(className && _objv_classs){
        
        objv_mutex_lock(& _objv_classs_mutex);
        
        objv_class_t * clazz = (objv_class_t *) objv_hash_map_get(_objv_classs, (void *) className);
        
        objv_mutex_unlock(& _objv_classs_mutex);
     
        return clazz;
    }
    
    return NULL;
}


void objv_class_initialize(objv_class_t * clazz){
   
    if(clazz && ! clazz->initialized){
        
        if(clazz->superClass){
            objv_class_initialize(clazz->superClass);
        }
        
        if(clazz->propertyCount >0){
            
            clazz->propertysMap = objv_hash_map_alloc(clazz->propertyCount, objv_hash_map_hash_code_key, objv_map_compare_key);
            
            {
                unsigned int c = clazz->propertyCount;
                objv_property_t * prop = clazz->propertys;
                
                while (c > 0 && prop) {
                    
                    objv_hash_map_put(clazz->propertysMap, (void *) prop->name, prop);
                    
                    c --;
                    prop ++;
                }
            }
            
        }
        
        if(clazz->methodCount >0){
            
            clazz->methodsMap = objv_hash_map_alloc(clazz->propertyCount, objv_hash_map_hash_code_key, objv_map_compare_key);
            
            {
                unsigned int c = clazz->methodCount;
                objv_method_t * method = clazz->methods;
                
                while (c > 0 && method) {
                    
                    objv_hash_map_put(clazz->methodsMap, (void *) method->name, method);
                    
                    c --;
                    method ++;
                }
            }
            
        }
    
        if(clazz->initialize){
            (* clazz->initialize)(clazz);
        }
        else {
            objv_object_class_initialize(clazz);
        }
        
        clazz->initialized = objv_true;
    }
}

objv_method_t * objv_class_getMethod(objv_class_t * clazz,objv_key_t * name){
    
    if(clazz && name){
        return objv_hash_map_get(clazz->methodsMap, (void *) name);
    }
    
    return NULL;
}

objv_method_t * objv_class_getMethodOfClass(objv_class_t * clazz,objv_key_t * name,objv_class_t ** ofClass){
    if(clazz && name){
        objv_class_t * c = clazz;
        objv_method_t * method = NULL;
        
        while (c && (method = objv_class_getMethod(c, name)) == NULL) {
            c = c->superClass;
        }
        
        if(ofClass){
            * ofClass = c;
        }
        
        return method;
    }
    return NULL;
}

objv_property_t * objv_class_getProperty(objv_class_t * clazz,objv_key_t * name){
    
    if(clazz && name){
        
        return objv_hash_map_get(clazz->propertysMap, (void *) name);
        
    }
    
    return NULL;
}


objv_property_t * objv_class_getPropertyOfClass(objv_class_t * clazz,objv_key_t * name,objv_class_t ** ofClass){
    if(clazz && name){
        objv_class_t * c = clazz;
        objv_property_t * prop = NULL;
        
        while (c && (prop = objv_class_getProperty(c, name)) == NULL) {
            c = c->superClass;
        }
        
        if(ofClass){
            * ofClass = c;
        }
        
        return prop;
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

objv_object_t * objv_object_init(objv_class_t * clazz,objv_object_t * object,...){
    va_list ap;
    objv_object_t * o;
    
    va_start(ap, object);
    
    o = objv_object_initv(clazz,object,ap);
    
    va_end(ap);
    
    return o;
}

objv_object_t * objv_object_initv(objv_class_t * clazz,objv_object_t * object,va_list ap){
    
    if(clazz && object){
        
        objv_class_t * c = clazz;
        
        objv_method_t * method = NULL;
        
        while(c && (method = objv_class_getMethod(c, OBJV_KEY(init))) == NULL){
            
            c = c->superClass;
        }
        
        if(method){
            return (* (objv_object_method_init_t) method->impl)(c,object,ap);
        }
    }
    
    return object;
}

objv_object_t * objv_object_allocv(objv_zone_t * zone,objv_class_t * clazz,va_list ap){
    return objv_object_alloc_exertv(zone,clazz,0,ap);
}

objv_object_t * objv_object_alloc_exertv(objv_zone_t * zone,objv_class_t * clazz,size_t exert,va_list ap){
    
    
    if(zone == NULL){
        zone = objv_zone_default();
    }
    
    objv_class_initialize(clazz);
    
    if(clazz && clazz->size > 0){
        
        objv_object_t * obj = (objv_object_t *) objv_zone_malloc(zone, clazz->size + exert);
        
        objv_zone_memzero(zone, obj, clazz->size);
        
        obj->isa = clazz;
        obj->zone = zone;
        obj->retainCount = 1;
        objv_mutex_init(& obj->mutex);
        
        return objv_object_initv(clazz, obj, ap);
    }
    
    return NULL;

    
}

objv_object_t * objv_object_alloc_exert(objv_zone_t * zone,objv_class_t * clazz,size_t exert,...){
    va_list ap;
    objv_object_t * object = NULL;
    
    va_start(ap, exert);
    
    object = objv_object_alloc_exertv(zone,clazz,exert,ap);
    
    va_end(ap);
    
    return object;
}

objv_object_t * objv_object_alloc(objv_zone_t * zone,objv_class_t * clazz,...){
    
    va_list ap;
    objv_object_t * object = NULL;
    
    va_start(ap, clazz);
    
    object = objv_object_alloc_exertv(zone,clazz,0,ap);
    
    va_end(ap);
    
    return object;
}

#undef objv_object_retain

objv_object_t * objv_object_retain(objv_object_t * object){
    
    if(object){
        
        objv_zone_retain(object->zone, object);
        
        objv_mutex_lock(& object->mutex);
        
        assert(object->retainCount > 0);
        
        object->retainCount ++;
        
        objv_mutex_unlock(& object->mutex);
        

    }
    
    return object;
}

#undef objv_object_release

void objv_object_release(objv_object_t * object){
    
    if(object){
        
        objv_zone_release(object->zone, object);
        
        objv_mutex_lock(& object->mutex);
        
        assert(object->retainCount > 0);
        
        object->retainCount -- ;
        
        int dodealloc = object->retainCount == 0;
        
        objv_mutex_unlock(& object->mutex);

        if( dodealloc ){
            
            objv_object_dealloc(object->isa,object);
            
            objv_mutex_lock(& object->mutex);
            
            {
                objv_object_weak_t * weak = object->weak, * tweak;
                
                while (weak) {
                    * weak->object = NULL;
                    tweak = weak;
                    weak = weak->next;
                    objv_zone_free(object->zone, tweak);
                }
                
                object->weak = NULL;
            }
            
            objv_mutex_unlock(& object->mutex);
            
            objv_mutex_destroy(& object->mutex);
            
            objv_zone_free(object->zone, object);
            
        }
    }
    
}

objv_object_t * objv_object_weak(objv_object_t * object, objv_object_t ** toObject){
    if(object && toObject){
        
        objv_object_weak_t * weak = (objv_object_weak_t *) objv_zone_malloc(object->zone, sizeof(objv_object_weak_t));
        
        weak->object = toObject;
        weak->next = NULL;
        
        objv_mutex_lock(& object->mutex);
     
        if(object->weak){
            weak->next = object->weak;
            object->weak = weak;
        }
        else{
            object->weak = weak;
        }
        
        objv_mutex_unlock(& object->mutex);
    }
    return object;
}

void objv_object_unweak(objv_object_t * object, objv_object_t ** toObject){
    if(object && toObject){
        
        objv_object_weak_t * weak ,* pweak;
        
        objv_mutex_lock(& object->mutex);
        
        pweak = NULL;
        weak = object->weak;
        
        while (weak) {
            
            if(toObject == weak->object){
                
                if(pweak){
                    pweak->next = weak->next;
                    objv_zone_free(object->zone, weak);
                    weak = pweak->next;
                }
                else{
                    object->weak = weak->next;
                    objv_zone_free(object->zone, weak);
                    weak = object->weak;
                }
            }
            else{
                pweak = weak;
                weak = weak->next;
            }
        }
        
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

objv_object_t * objv_object_copy(objv_class_t * clazz,objv_object_t * object){
    
    if(clazz && object){
        
        objv_class_t * c = clazz;
        
        objv_method_t * method = NULL;
        
        while(c && (method = objv_class_getMethod(c, OBJV_KEY(copy))) == NULL){
            
            c = c->superClass;
        }
        
        if(method){
            return (* (objv_object_method_copy_t) method->impl)(c,object);
        }
        
    }
    
    return NULL;
}


