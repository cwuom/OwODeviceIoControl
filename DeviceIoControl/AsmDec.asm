public	AsmDec800

.code _text
AsmDec800 proc
    vmovaps ymm15, [rdx]
    vmovaps xmm0,xmm11             
    vmovaps xmm1,xmm12             
    vmovaps xmm2,xmm13             
    vmovaps xmm3,xmm14             
    vextractf128 xmm14,ymm15,1h     
    vpunpckhqdq xmm14,xmm14,xmm14  
    vpshufd xmm12,xmm14,0h          
    vpshufd xmm13,xmm14,55h         
    vpsrld xmm14,xmm13,7h           
    vpslld xmm13,xmm13,19h          
    vpor xmm13,xmm13,xmm14         
    vpslld xmm14,xmm12,3h           
    vpsrld xmm11,xmm12,1Dh          
    vpor xmm14,xmm14,xmm11         
    vxorps xmm12,xmm12,xmm13       
    vpaddd xmm12,xmm12,xmm14       
    vpslld xmm14,xmm12,0Dh           
    vpsrld xmm11,xmm12,13h          
    vpor xmm14,xmm14,xmm11         
    vpsrld xmm13,xmm12,0Bh           
    vpslld xmm11,xmm12,15h          
    vpor xmm13,xmm13,xmm11         
    vmovd xmm11,ecx                
    vpshufd xmm11,xmm11,0h          
    vxorps xmm11,xmm11,xmm13       
    vmovdqa xmm12,xmm11            
    vpslld xmm11,xmm11,5h           
    vpsrld xmm12,xmm12,1Bh          
    vpor xmm11,xmm11,xmm12         
    vpsubd xmm11,xmm11,xmm14       
    vmovd eax,xmm11                
    mov rdx,0FFFFFFFF00000000h       
    and rcx,rdx  
    or rax,rcx                     
    vmovaps xmm14,xmm3             
    vmovaps xmm13,xmm2             
    vmovaps xmm12,xmm1             
    vmovaps xmm11,xmm0             
    ret                            
AsmDec800 endp


end