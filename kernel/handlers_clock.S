# cette directive sert a rendre l'etiquette publique
    .globl handler_IT_32
# debut du traitant
handler_IT_32:
# sauvegarde des registres importants
    pushl %eax
    pushl %edx
    pushl %ecx
# appel a la fonction C realisant le traitant
    call clock_interrupt_handler
# restauration des registres importants
    popl %ecx
    popl %edx
    popl %eax
# fin du traitant
    iret
