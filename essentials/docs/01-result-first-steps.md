# Lesson 01 — Result: structs, enums, and a borrowed message

**Status: guided exercise; the implementation is intentionally absent.**

This is the first actual CBox API exercise. It starts with a diagnostic rather than asking the learner to reproduce an unfamiliar API by rote.

## 1. Diagnostic answers to understand first

```c
typedef enum
{
    RESULT_OK,
    RESULT_FAIL
} ResultStatus;

typedef struct
{
    ResultStatus status;
    int error_code;
} Result;
```

- `typedef` introduces another **name** for a type; it does not automatically create a new distinct nominal type.
- The result's **status** is the authoritative indicator of success. The numeric error code is additional information and may be zero even for a failure when no code is specified.
- Passing `Result r` to a function passes the **entire struct by value**. Reassigning members in the called function affects only its local copy.
- Passing `Result *r` passes a **copy of the pointer value**. The expression `r->status` means `(*r).status` and can change the pointed-to caller-owned struct.
- Returning a struct **by value** is normal C and does not require returning a pointer to a local object.

### String literal versus local character array

```c
/* Safe: the string literal has static storage duration. */
const char *message_a = "File not found";
```

Contrast:

```c
/* Unsafe if the returned pointer outlives the local array. */
const char *bad_message(void)
{
    char message[] = "File not found";
    return message; /* DO NOT USE AFTER RETURN */
}
```

Important: a returned struct containing a pointer does **not** automatically extend the lifetime of the object to which the pointer refers.

## 2. Our first, deliberately small contract

Study `essentials/include/cbox/essentials/result.h`. It currently declares:

```c
typedef enum
{
    CBOX_RESULT_UNKNOWN = 0,
    CBOX_RESULT_OK = 1,
    CBOX_RESULT_FAIL = 2
} cbox_result_status;

typedef struct
{
    cbox_result_status status;
    int error_code;
    const char *message;
} cbox_result;

cbox_result cbox_result_ok(void);
cbox_result cbox_result_fail(int error_code, const char *message);
```

We deliberately **do not use a union** yet: there is no success payload to share storage with error fields, and the failure message is optional. Response will have a separate design when this basic type works.

### Invariants

- `cbox_result_ok()` returns status `CBOX_RESULT_OK`, code 0, message `NULL`.
- `cbox_result_fail(404, "File not found")` returns status `CBOX_RESULT_FAIL`, code 404, and a pointer to that message.
- `cbox_result_fail(0, NULL)` is also valid: failure with no code or description.
- `status` tells success vs failure; **do not infer status from error_code**.
- A zero-initialized result has status `UNKNOWN`, not `OK`.
- `message` is a **borrowed** pointer: CBox does not allocate, copy, or free the text. A string literal is a safe first example. A pointer to a local buffer that has gone out of scope is not.

This simple model is a provisional C17 learning API. After tests, we can compare it with richer Toolbox status semantics and decide how to evolve it before a stable release.

## 3. Test-first: run the deliberate RED stage

From the CBox repository root:

```bash
git pull --ff-only
make test
make test-result
```

`make test` runs the *existing* C17 smoke test and should still pass.

`make test-result` is the new **test-first** target. Before you create `essentials/src/result.c`, it is expected to fail because that source file does not exist. This is intentional and does not indicate that the compiler or Git clone is broken.

The unit tests already exist in `essentials/tests/test_result.c`.

## 4. YOUR implementation: create result.c

Create `essentials/src/result.c`. Start from this incomplete skeleton:

```c
#include <stddef.h>

#include "cbox/essentials/result.h"

cbox_result cbox_result_ok(void)
{
    /* TODO: create a local cbox_result with a successful status,
       zero error code, and NULL message; return it by value. */
}

cbox_result cbox_result_fail(int error_code, const char *message)
{
    /* TODO: create a local cbox_result with a failed status,
       the caller's error code, and borrowed message pointer.
       Return the struct by value. */
}
```

We have purposefully not filled in the function bodies. Predict how the two resulting objects should look before writing them.

One possible implementation style is to create a local variable, initialize or assign all its members, and `return result;`.

**Do not use `malloc()` or `free()`.** Nothing in these two constructors needs dynamic allocation.

## 5. GREEN stage: compile and run tests

```bash
make test-result
```

Expected **after implementing both functions correctly**:

```text
CBox Essentials Result: all initial tests passed.
```

The new target compiles the unit test and your source file with the existing C17 warning flags. The existing `make test` remains a separate toolchain check at this stage; we will combine them once the new Result implementation is tested.

If a warning or error appears, read its filename and line number. Fix the first underlying problem rather than suppressing warnings.

## 6. Understand the returned value

These are different operations:

```c
cbox_result r = cbox_result_fail(5, "Bad input");
cbox_result copy = r;
```

The entire struct is copied. Its two pointer values (if any) refer to the **same** text. There is no deep copy of the message.

The caller must never do this to a message borrowed from a literal:

```c
/* WRONG: message was not allocated by this API. */
free((void *)r.message);
```

Nor should it return a borrowed pointer to an automatic character array that goes out of scope.

Later we can design owning response buffers and explicit cleanup functions. We will first learn exactly what a borrowed pointer guarantees and what it does not guarantee.

## 7. Follow-up exercises before expanding API

1. What is the status of `cbox_result blank = {0};` and why?
2. Is `cbox_result_fail(0, NULL)` a success?
3. Can `cbox_result_ok()` be returned directly from a function without allocating memory?
4. Does copying `cbox_result` copy the error message's characters?
5. Which component should depend on which: `essentials -> when_it_fails` or `when_it_fails -> essentials`?

## 8. Next integration steps

After the learner has working tests:

1. Include the Result unit test in the normal `make test` workflow.
2. Add negative and boundary checks, including message pointer lifetime examples where applicable.
3. Add one genuinely useful MemView operation returning `cbox_result`.
4. Document stable status semantics, versioning, and shared behavioral fixtures.
5. Only then start the separate Response design.

The lesson is part of the repository, but it does not automatically modify MemView or any other consumer.
