//
//  objv_cloud.h
//  objv
//
//  Created by zhang hailong on 14-2-9.
//  Copyright (c) 2014年 hailong.org. All rights reserved.
//

#ifndef objv_objv_cloud_h
#define objv_objv_cloud_h



#ifdef __cplusplus
extern "C" {
#endif
    
#include "objv_array.h"
#include "objv_dispatch.h"
#include "objv_string.h"
#include "objv_hash_map.h"
    
    typedef unsigned long long CLIdentifier;
    
    typedef struct _CLContext {
        objv_object_t base;
        objv_string_t * READONLY domain;
        CLIdentifier READONLY identifier;
        objv_dispatch_t * READONLY dispatch;
        struct _CLContext * READONLY parent;
        objv_array_t * READONLY childs;
        objv_object_t * READONLY config;
    } CLContext;
    
    OBJV_KEY_DEC(CLContext)
    OBJV_CLASS_DEC(CLContext)
    
    typedef struct _CLService {
        objv_object_t base;
        objv_object_t * READONLY config;
        objv_hash_map_t * READONLY taskTypes;
        objv_boolean_t inherit;
    } CLService;
    
    OBJV_KEY_DEC(CLService)
    OBJV_CLASS_DEC(CLService)
    

    
    typedef struct _CLTask {
        objv_object_t base;
        CLIdentifier READONLY identifier;
        CLIdentifier READONLY replyIdentifier;
        CLContext * READONLY source;
        objv_string_t * READONLY target;
    } CLTask;
    
    OBJV_KEY_DEC(CLTask)
    OBJV_CLASS_DEC(CLTask)
    
    
    typedef struct _CLServiceContainer {
        objv_object_t base;
        objv_array_t * READONLY services;
        objv_object_t * READONLY config;
    } CLServiceContainer;
    
    OBJV_KEY_DEC(CLServiceContainer)
    OBJV_CLASS_DEC(CLServiceContainer)
    
    void CLServiceContainerAddService(CLServiceContainer * container,CLService * service);
    
    void CLServiceContainerSetConfig(CLServiceContainer * container,objv_object_t * config);
    
    void CLServiceContainerHandleTask(CLServiceContainer * container,CLContext * ctx, objv_class_t * taskType,CLTask * task);
    
    typedef objv_boolean_t ( * CLServicetHandleTaskFun ) (objv_class_t * clazz,CLService * service,CLContext * ctx,objv_class_t * taskType,CLTask * task);
    
    void CLServiceSetConfig(CLService * service, objv_object_t * config);
    
    void CLServiceAddTaskType(CLService * service,objv_class_t * taskType);
    
    objv_boolean_t CLServiceHasTaskType(CLService * service,objv_class_t * taskType);
    
    objv_boolean_t CLServiceHandleTask(CLService * service,CLContext * ctx,objv_class_t * taskType,CLTask * task);
    
    OBJV_KEY_DEC(handleTask)
    OBJV_KEY_DEC(sendTask)
    
    
    typedef void ( * CLContextHandleTaskFun ) (objv_class_t * clazz,CLContext * context,objv_class_t * taskType,CLTask * task);
    typedef void ( * CLContextSendTaskFun ) (objv_class_t * clazz,CLContext * context,objv_class_t * taskType,CLTask * task);
    
    void CLContextHandleTask(CLContext * context, objv_class_t * taskType, CLTask * task);
    
    void CLContextSendTask(CLContext * context, objv_class_t * taskType, CLTask * task);
    
    void CLContextAddChild(CLContext * context, CLContext * child);
    
    
    OBJV_KEY_DEC(setConfig)
    
    typedef void ( * CLContextSetConfigFun ) (objv_class_t * clazz,CLContext * context,objv_object_t * config);
    
    void CLContextSetConfig(objv_class_t * clazz, CLContext * context,objv_object_t * config);
    
#ifdef __cplusplus
}
#endif



#endif