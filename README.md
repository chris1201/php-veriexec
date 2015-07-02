
#Usage#

Add the following to the [Zend] section of php.ini:

```
zend_extension=/path/to/veriexec.so

; location of the signature database
zend.veriexec_file="sigs.dat"

; the verification mode requested, currently:
; 3 = learn (Will APPEND unknown signatures to DB automatically upon execution)
; 2 = warn - Only emit PHP Warning if unknown code is seen
; 1 = refuse - Emit PHP Warning AND refuse to execute unknown code
; 0 = halt  - Emit PHP Error about unknown code AND exit script
zend.veriexec_mode=1
```


