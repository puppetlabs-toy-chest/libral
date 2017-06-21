#
# A simple completion function for ralsh. Only completes ralsh TYPE NAME
# but can't add attributes.
#
__ralsh_complete()
{
    if [[ ( "$3" == "-I" ) || ( "$3" == "--include" ) ]]
    then
        # completing the argument of -I/--include
        COMPREPLY=( $(compgen -o dirnames "$2") )
        return
    fi
    echo "w: ${COMP_WORDS[@]}"
    type=${COMP_WORDS[1]}
    type_pat="${type}::"
    if [[ ( $COMP_CWORD -ge 4 ) && ( ${COMP_WORDS[2]} == "::") ]]
    then
       type="${type}::${COMP_WORDS[3]}"
       type_pat="$type"
    fi

    if [[ $COMP_CWORD -eq 1 ]]
    then
        COMPREPLY=( $(compgen -W "$(ralsh)" -- "$2") )
    else
        n=$(${COMP_LINE} "$type" | grep "$type_pat" | sed -r -e "s/^.* \{ '//" -e "s/':.*$//")
        COMPREPLY=( $(compgen -W "$n" -- "$2") )
    fi
}

complete -F __ralsh_complete ralsh
