c ==========================================================
c subroutine dmultifft computes cross-correlation function
c            with the double precision accuracy.
c input:
c len   - double the length of the input (integer*4)
c n1    - number of samples in arrays f1, (integer*4)
c f1amp - first input arrays amp (real*4)
c f1phase  - first input arrays phase (real*4)
c n2    - number of samples in arrays f2, (integer*4)
c f2amp - second input arraysamp (real*4)
c f2phase- second input arraysphase (real*4)
c lag   - lag the cross-correlation function, (integer*4)
c       - lag < max(n1,n2)/2
c output:
c cor   - cross-correlation array with lag*2+1 samples, (real*4)
c ==========================================================
      subroutine dmultifft(len, amp2, phase2, amp, phase, lag, seis_out,ns)
      implicit none
      integer*4 len,ns
      real*4    amp(len),phase(len),amp2(len),phase2(len)
      include 'fftw3.f'
      integer*4 lag, i,k
c      real*4    cor(10001)
      real*8    plan3
      double complex temp1(400000),temp2(400000)
      double complex temp3(400000),czero
      real*4  seis_out(400000)

cxx   common /core/temp1

      czero = (0.0d0,0.0d0)

c read amp and phase into complex temp2
 
       do i = 1,len
          temp1(i) = dcmplx(dble(amp2(i)),dble(phase2(i)))
          temp2(i) = dcmplx(dble(amp(i)),dble(phase(i)))
cxx       temp2(i) = dcmplx(amp(i)*cos(phase(i)),
cxx  1                      amp(i)*sin(phase(i)))
       enddo

c multiply to get correlation in freq domain
      do i =1,len
          temp3(i) = temp1(i)*dconjg(temp2(i))
      enddo

      ns = (len-1)*2
      do k = len+1,ns
         temp3(k) = czero
      enddo


c compute ifft 
      call dfftw_plan_dft_1d(plan3,ns,temp3,temp2,
     *                         FFTW_FORWARD, FFTW_ESTIMATE)
      call dfftw_execute(plan3)
      call dfftw_destroy_plan(plan3)
c extract correlatio

      do k = 1,ns
        seis_out(k) = 2.0*real(dreal(temp2(k)))/ns
      enddo

      return

      end
