# DnsResolver

My attempt to build a toy DNS resolver in c++. Reference: https://implement-dns.wizardzines.com/book/part_1

## Learnings

- Google public DNS resides on 8.8.8.8
- port 53 is DNS port
- `void *memset(void *str, int c, size_t n)` copies the character `c` (an unsigned char) to the first `n` characters of the string pointed to, by the argument `str`
- `void *calloc(size_t nitems, size_t size)` allocates the requested memory and returns a pointer to it. The difference in malloc and calloc is that malloc does not set the memory to zero where as calloc sets allocated memory to zero
- Given a human readable domain name, such as www.charity.org, the string is broken apart into distinct fields based on the dot; in this case, the three fields are "www", "charity", and "org". Within the DNS question, each field is preceded by a one-byte value that indicates the length of the field. The name is considered terminated once the null-byte is used to indicate a zero-length field
