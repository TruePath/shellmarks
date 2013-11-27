
# The outcommented alias does not supplant cd, the other one does.
# alias to    'cd `(apparix -favour rOl \!* || echo -n .)`'
alias to '(test "x-" =  "x\!*") && cd - || (test "x" !=  "x\!*") && cd `(apparix --try-current-first -favour rOl \!* || echo -n .)` || cd'

alias bm   'apparix --add-mark \!*'
alias portal 'apparix --add-portal \!*'
alias als '(test "x" !=  "x\!*") && ls `(apparix --try-current-first -favour rOl \!* || echo -n /dev/null)`'

# if the bookmark does not exist this will still fire up $EDITOR.
alias ae '(test "x" !=  "x\!*") && $EDITOR `(apparix --try-current-first -favour rOl \!*)`'

