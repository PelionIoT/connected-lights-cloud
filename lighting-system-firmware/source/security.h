/*
 * Copyright (c) 2015 ARM Limited. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef __SECURITY_H__
#define __SECURITY_H__
 
#include <inttypes.h>
 
#define MBED_DOMAIN "fc5bae18-72d8-4d16-a0d0-00608fa76464"
#define MBED_ENDPOINT_NAME "8874e0eb-96ef-4799-b948-e91d05147bfe"
 
const uint8_t SERVER_CERT[] = "-----BEGIN CERTIFICATE-----\r\n"
"MIIBmDCCAT6gAwIBAgIEVUCA0jAKBggqhkjOPQQDAjBLMQswCQYDVQQGEwJGSTEN\r\n"
"MAsGA1UEBwwET3VsdTEMMAoGA1UECgwDQVJNMQwwCgYDVQQLDANJb1QxETAPBgNV\r\n"
"BAMMCEFSTSBtYmVkMB4XDTE1MDQyOTA2NTc0OFoXDTE4MDQyOTA2NTc0OFowSzEL\r\n"
"MAkGA1UEBhMCRkkxDTALBgNVBAcMBE91bHUxDDAKBgNVBAoMA0FSTTEMMAoGA1UE\r\n"
"CwwDSW9UMREwDwYDVQQDDAhBUk0gbWJlZDBZMBMGByqGSM49AgEGCCqGSM49AwEH\r\n"
"A0IABLuAyLSk0mA3awgFR5mw2RHth47tRUO44q/RdzFZnLsAsd18Esxd5LCpcT9w\r\n"
"0tvNfBv4xJxGw0wcYrPDDb8/rjujEDAOMAwGA1UdEwQFMAMBAf8wCgYIKoZIzj0E\r\n"
"AwIDSAAwRQIhAPAonEAkwixlJiyYRQQWpXtkMZax+VlEiS201BG0PpAzAiBh2RsD\r\n"
"NxLKWwf4O7D6JasGBYf9+ZLwl0iaRjTjytO+Kw==\r\n"
"-----END CERTIFICATE-----\r\n";
 
const uint8_t CERT[] = "-----BEGIN CERTIFICATE-----\r\n"
"MIIB0DCCAXOgAwIBAgIEes8ZqjAMBggqhkjOPQQDAgUAMDkxCzAJBgNVBAYTAkZ\r\n"
"JMQwwCgYDVQQKDANBUk0xHDAaBgNVBAMME21iZWQtY29ubmVjdG9yLTIwMTYwHh\r\n"
"cNMTYwNjEzMTMyMTE3WhcNMTYxMjMxMDYwMDAwWjCBoTFSMFAGA1UEAxNJZmM1Y\r\n"
"mFlMTgtNzJkOC00ZDE2LWEwZDAtMDA2MDhmYTc2NDY0Lzg4NzRlMGViLTk2ZWYt\r\n"
"NDc5OS1iOTQ4LWU5MWQwNTE0N2JmZTEMMAoGA1UECxMDQVJNMRIwEAYDVQQKEwl\r\n"
"tYmVkIHVzZXIxDTALBgNVBAcTBE91bHUxDTALBgNVBAgTBE91bHUxCzAJBgNVBA\r\n"
"YTAkZJMFkwEwYHKoZIzj0CAQYIKoZIzj0DAQcDQgAEXTg2g2Jp/F6fGwuf3ixi5\r\n"
"0oOPewjcXyYinuntvp+meGz3YI6HiB0QITjK35L79WQabVIIKa+SnKjiY8Y2XCk\r\n"
"9TAMBggqhkjOPQQDAgUAA0kAMEYCIQCMOmcD1D2t8l3Fx52s9ywIY+yLWVyHexV\r\n"
"W1bhgn6FA/wIhANYvHGHKIivoOlMJPeeSrEvG6IOL5CddklJP1SMjq+nm\r\n"
"-----END CERTIFICATE-----\r\n";
 
const uint8_t KEY[] = "-----BEGIN PRIVATE KEY-----\r\n"
"MIGHAgEAMBMGByqGSM49AgEGCCqGSM49AwEHBG0wawIBAQQga6dP1XKBu+LoAyu8\r\n"
"qatcKKwoodW6DJNd9OR03EQH9vWhRANCAARdODaDYmn8Xp8bC5/eLGLnSg497CNx\r\n"
"fJiKe6e2+n6Z4bPdgjoeIHRAhOMrfkvv1ZBptUggpr5KcqOJjxjZcKT1\r\n"
"-----END PRIVATE KEY-----\r\n";
 
#endif //__SECURITY_H__
