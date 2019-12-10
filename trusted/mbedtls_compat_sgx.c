/*
 * (C) Copyright 2016 Fan Zhang (bl4ck5unxx@gmail.com)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Authors:
 *     Fan Zhang <bl4ck5unxx@gmail.com>
 *     Fábio Silva <fabio.fernando.osilva@gmail.com>
 *
 * This is a glue file calling ocalls.
 */

#include <stdarg.h>
#include <stdio.h> /* vsnprintf */
#include "mbedtls_compat_sgx.h"
#include "sgx.h"
#include "sgx_trts.h"

// real ocall to be implemented in the Application
extern int ocall_mbedtls_compat_sgx_print_string(int *ret, char *str);
int mbedtls_compat_sgx_printf(const char *fmt, ...)
{
    int ret;
    va_list ap;
    char buf[BUFSIZ] = {'\0'};
    va_start(ap, fmt);
    vsnprintf(buf, BUFSIZ, fmt, ap);
    va_end(ap);

    ocall_mbedtls_compat_sgx_print_string(&ret, buf);
    return ret;
}

// ocall for entropy collection
int mbedtls_hardware_poll(void *data, unsigned char *output, size_t len, size_t *olen)
{
    (void)data;
    sgx_status_t st = sgx_read_rand(output, len);
    if (st != SGX_SUCCESS)
    {
        mbedtls_compat_sgx_printf("hardware_poll fails with %d\n", st);
        *olen = -1;
        return -1;
    }
    else
    {
        *olen = len;
        return 0;
    }
}

// ocall for hardware RNG
int mbedtls_sgx_drbg_random(void *p_rng, unsigned char *output, size_t out_len)
{
    if (!output)
    {
        mbedtls_compat_sgx_printf("mbedtls_sgx_drbg receives NULL");
        return -1;
    }
    (void)p_rng;
    sgx_status_t st = sgx_read_rand(output, out_len);
    if (st != SGX_SUCCESS)
    {
        mbedtls_compat_sgx_printf("mbedtls_sgx_drbg fails with %d\n", st);
        return -1;
    }

    return 0;
}

// ocall for rand implementation
int rand()
{
    int random_val = 0;

    sgx_status_t ret = sgx_read_rand((unsigned char *)&random_val, sizeof(random_val));
    if (SGX_SUCCESS != ret)
    {
        mbedtls_compat_sgx_printf("rand fails with %d\n", SGX_SUCCESS);
        return -1;
    }
    return random_val;
}
