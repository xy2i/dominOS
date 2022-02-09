    .text
# cette directive sert a rendre l'etiquette publique
    .globl handler_IT_32
# debut du handler
handler_IT_32:
# sauvegarde des registres importants
    pushl %eax
    pushl %edx
    pushl %ecx
# appel a la fonction C realisant le handler
# a decommenter quand vous l'aurez implantee
    call tic_PIT
# restauration des registres importants
    popl %ecx
    popl %edx
    popl %eax
# fin du handler
    iret
