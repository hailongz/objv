//
//  objv_mbuf.c
//  objv
//
//  Created by zhang hailong on 14-2-3.
//  Copyright (c) 2014年 hailong.org. All rights reserved.
//

#define READONLY

#include "objv_os.h"
#include "objv.h"
#include "objv_mbuf.h"
#include "objv_value.h"

#define EXTEND  64
#define NUMBER_SIZE 64

void objv_mbuf_init(objv_mbuf_t * mbuf,size_t capacity){
    objv_zone_memzero(NULL,mbuf, sizeof(objv_mbuf_t));
    mbuf->size = capacity;
    mbuf->data = objv_zone_malloc(NULL, capacity);
}

void objv_mbuf_destroy(objv_mbuf_t * mbuf){
    if(mbuf->data){
        objv_zone_free(NULL, mbuf->data);
    }
    objv_zone_memzero(NULL,mbuf, sizeof(objv_mbuf_t));
}

void objv_mbuf_extend(objv_mbuf_t * mbuf,size_t size){
    if(size > mbuf->size){
        mbuf->size = size;
        mbuf->data = objv_zone_realloc(NULL, mbuf->data, mbuf->size);
    }
}

void objv_mbuf_append(objv_mbuf_t * mbuf,void * bytes,size_t length){
    
    if(mbuf->length + length + 1 > mbuf->size){
        mbuf->size = mbuf->length + length + EXTEND;
        mbuf->data = objv_zone_realloc(NULL, mbuf->data, mbuf->size);
    }
    
    memcpy((char *) mbuf->data + mbuf->length, bytes, length);
    
    mbuf->length += length;
    
}

void objv_mbuf_formatv(objv_mbuf_t * mbuf,const char * format,va_list va){
    char text[NUMBER_SIZE];
    char fmt[NUMBER_SIZE];
    char * p = (char *) format;
    int s = 0,fIndex = 0,len;
    union {
        int intValue;
        long longValue;
        long long longLongValue;
        unsigned int uintValue;
        unsigned long ulongValue;
        unsigned long long ulongLongValue;
        double doubleValue;
    } value;
    
    while (p && *p != 0) {
        
        if(s == 0){
            
            if(*p == '%'){
                s = 1;
                fIndex = 0;
                assert(fIndex + 2 < NUMBER_SIZE);
                fmt[fIndex ++] = * p;
            }
            else{
                objv_mbuf_append(mbuf, p, 1);
            }
        }
        else {
            if( *p == '@'){
                {
                    objv_string_t * v = objv_object_stringValue(va_arg(va, objv_object_t *), NULL);
                    if(v){
                        objv_mbuf_append(mbuf, (void *) v->UTF8String, v->length);
                    }
                    s = 0;
                }
            }
            else if( *p == 'd'){
                {
                    assert(fIndex + 2 < NUMBER_SIZE);
                    fmt[fIndex ++] = * p;
                    fmt[fIndex] = 0;
                    
                    if(p[-1] == 'l' && p[-2] == 'l'){
                        value.longLongValue = va_arg(va, long long);
                        len = snprintf(text, sizeof(text),fmt,value.longLongValue);
                    }
                    else if(p[-1] == 'l'){
                        value.longValue = va_arg(va, long);
                        len = snprintf(text, sizeof(text),fmt,value.longValue);
                    }
                    else {
                        value.intValue = va_arg(va, int);
                        len = snprintf(text, sizeof(text),fmt,value.intValue);
                    }
                    
                    objv_mbuf_append(mbuf, text, len);
                    
                    s = 0;
                }
            }
            else if( *p == 'u'){
                {
                    assert(fIndex + 2 < NUMBER_SIZE);
                    fmt[fIndex ++] = * p;
                    fmt[fIndex] = 0;
                    
                    if(p[-1] == 'l' && p[-2] == 'l'){
                        value.ulongLongValue = va_arg(va,unsigned long long);
                        len = snprintf(text, sizeof(text),fmt,value.ulongLongValue);
                    }
                    else if(p[-1] == 'l'){
                        value.ulongValue = va_arg(va,unsigned long);
                        len = snprintf(text, sizeof(text),fmt,value.ulongValue);
                    }
                    else {
                        value.uintValue = va_arg(va,unsigned int);
                        len = snprintf(text, sizeof(text),fmt,value.uintValue);
                    }
                    
                    objv_mbuf_append(mbuf, text, len);
                    
                    s = 0;
                }
            }
            else if( *p == 'x'){
                {
                    assert(fIndex + 2 < NUMBER_SIZE);
                    fmt[fIndex ++] = * p;
                    fmt[fIndex] = 0;
                    
                    value.uintValue = va_arg(va,unsigned int);
                    len = snprintf(text, sizeof(text),fmt,value.uintValue);
                    
                    objv_mbuf_append(mbuf, text, len);
                    
                    s = 0;
                }
            }
            else if( *p == 'X'){
                {
                    assert(fIndex + 2 < NUMBER_SIZE);
                    fmt[fIndex ++] = * p;
                    fmt[fIndex] = 0;
                    
                    value.uintValue = va_arg(va,unsigned int);
                    len = snprintf(text, sizeof(text),fmt,value.uintValue);
                    
                    objv_mbuf_append(mbuf, text, len);
                    
                    s = 0;
                }
            }
            else if( *p == 'f'){
                {
                    assert(fIndex + 2 < NUMBER_SIZE);
                    fmt[fIndex ++] = * p;
                    fmt[fIndex] = 0;
                    
                    value.doubleValue = va_arg(va,double);
                    len = snprintf(text, sizeof(text),fmt,value.doubleValue);
                    
                    objv_mbuf_append(mbuf, text, len);
                    
                    s = 0;
                }
            }
            else if( *p == 's'){
                {
                    char * svalue = va_arg(va, char *);
                    if(svalue){
                        objv_mbuf_append(mbuf, svalue, strlen(svalue));
                    }
                    s = 0;
                }
            }
            else if( *p == '%'){
                {
                    objv_mbuf_append(mbuf, p, 1);
                    s = 0;
                }
            }
            else{
                assert(fIndex + 2 < NUMBER_SIZE);
                fmt[fIndex ++] = * p;
            }
        }
        
        p ++;
    }
    
}

void objv_mbuf_format(objv_mbuf_t * mbuf,const char * format,...){
    
    va_list ap;
    
    va_start(ap, format);
    
    objv_mbuf_formatv(mbuf, format, ap);
    
    va_end(ap);
    
}

const char * objv_mbuf_str(objv_mbuf_t * mbuf){
    char * p = (char *) mbuf->data;
    objv_mbuf_extend(mbuf, mbuf->length + 1);
    p[mbuf->length] = 0;
    return p;
}

void objv_mbuf_clear(objv_mbuf_t * mbuf){
    mbuf->length = 0;
}

