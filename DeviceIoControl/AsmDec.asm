public	AsmDec800

.data
align 16
ymm15_save db 32 dup(0)  ; YMM15保存空间（32字节）
align 16  
xmm11_save db 16 dup(0)  ; XMM11保存空间（16字节）
xmm12_save db 16 dup(0)  ; XMM12保存空间（16字节）
xmm13_save db 16 dup(0)  ; XMM13保存空间（16字节）
xmm14_save db 16 dup(0)  ; XMM14保存空间（16字节）

.code
AsmDec800 proc
    ; 保存YMM15寄存器
    vmovaps ymmword ptr [ymm15_save], ymm15
    
    ; 保存XMM寄存器
    vmovaps xmmword ptr [xmm11_save], xmm11
    vmovaps xmmword ptr [xmm12_save], xmm12  
    vmovaps xmmword ptr [xmm13_save], xmm13
    vmovaps xmmword ptr [xmm14_save], xmm14

    ; 算法主体（保持不变）
    vmovaps ymm15, [rdx]   
    
    vextractf128 xmm14, ymm15, 1h     
    vpunpckhqdq xmm14, xmm14, xmm14  
    vpshufd xmm12, xmm14, 0h          
    vpshufd xmm13, xmm14, 55h         
    vpsrld xmm14, xmm13, 7h           
    vpslld xmm13, xmm13, 19h          
    vpor xmm13, xmm13, xmm14         
    vpslld xmm14, xmm12, 3h           
    vpsrld xmm11, xmm12, 1Dh          
    vpor xmm14, xmm14, xmm11         
    vxorps xmm12, xmm12, xmm13       
    vpaddd xmm12, xmm12, xmm14       
    vpslld xmm14, xmm12, 0Dh           
    vpsrld xmm11, xmm12, 13h          
    vpor xmm14, xmm14, xmm11         
    vpsrld xmm13, xmm12, 0Bh           
    vpslld xmm11, xmm12, 15h          
    vpor xmm13, xmm13, xmm11         
    vmovd xmm11, ecx                
    vpshufd xmm11, xmm11, 0h          
    vxorps xmm11, xmm11, xmm13       
    vmovdqa xmm12, xmm11            
    vpslld xmm11, xmm11, 5h           
    vpsrld xmm12, xmm12, 1Bh          
    vpor xmm11, xmm11, xmm12         
    vpsubd xmm11, xmm11, xmm14       
    vmovd eax, xmm11                
    mov rdx, 0FFFFFFFF00000000h       
    and rcx, rdx  
    or rax, rcx                     

    ; 恢复寄存器（按保存的逆序）
    vmovaps xmm11, xmmword ptr [xmm11_save]
    vmovaps xmm12, xmmword ptr [xmm12_save]
    vmovaps xmm13, xmmword ptr [xmm13_save]  
    vmovaps xmm14, xmmword ptr [xmm14_save]
    
    ; 恢复YMM15寄存器
    vmovaps ymm15, ymmword ptr [ymm15_save]
    
    ret                            
AsmDec800 endp
end


end