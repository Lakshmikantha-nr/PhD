p0 = 0.437;         # -, total porosity                                                
p0e = 0.401;        # -, drainable porosity           
Se = 0.1;           # -, initial effective saturation <==== ANTECEDENT MOISTURE          
psi = 6.13;        # cm, suction head                             
K = 2.99;           # cm/hr, saturated hydraulic conductivity                            
hstor = 0.5;       # cm, depression storage capacity                                    

dtheta0 <- p0e*(1-Se)# -, moisture capacity   
getwd()
setwd("C:/Users/ATREE/Desktop/IPCClead")
getwd()
pathname = "C:/Users/ATREE/Desktop/IPCClead/"
filename =  "dummyRF.csv"
file = paste(pathname,filename, sep="")
H <- read.csv(file,header=TRUE)                                                          
names(H) = c("minutes","rainfall")
H
idt <- H$rainfall
#idt <- diff(H$rainfall)          # uncomment this and the line below 
# if precip is provided as cumulative rainfall depth.                
#idt <- c(0,idt)   # cm, rainfall during time increment"                                  
nt <- length(H$minutes)                                                                         
dt <- H$minutes[2]-H$minutes[1]

fp <- rep(0,nt)
dFa <- rep(0,nt)
Fp <- rep(0,nt)
dFp <- rep(0,nt)              
Sstor <- rep(0,nt)
exc <- rep(0,nt)
runoff <- rep(0,nt)                           
dSstor <- rep(0,nt)
dexc <- rep(0,nt)
idt_plus_Sstor <- rep(0,nt)

Fp[1] <- 0.00001# need nonzero value since Fp is in denominator of fp eqn.

for (i in 2:nt){                                                                          
  fp[i] <- K*(1 + (psi*dtheta0)/Fp[i-1])      # pot. inf. rate  <======
  dFp[i] <- fp[i]*dt/60                       # pot. cumul. inf.                        
  idt_plus_Sstor[i] <- idt[i]+Sstor[i-1]      # water needing entry                           
  dFa[i] <- min(dFp[i],idt_plus_Sstor[i])     # actual inf. increment                      
  Fp[i] <- Fp[i-1] + dFa[i]                   # actual infilt. <========                                   
  dexc[i] <- idt_plus_Sstor[i] - dFa[i]       # excess rainfall                                
  Sstor[i] <- min(dexc[i],hstor)              # depression storage                                    
  runoff[i] <- dexc[i] - Sstor[i]             # actual runoff                                        
}

dFahr <- dFa*4

par(mfrow=c(2,2),mar=c(1,4,0,4))
plot(H$minutes,idt,type="S", ylab = "rainfall, cm", xaxt="n")
axis(side=1,labels=FALSE)
plot(H$minutes,dFahr,type="b", ylab = "infiltration rate, cm/hr", xaxt="n")
axis(side=1,labels=FALSE)
plot(H$minutes,Fp,type="b", ylab = "infiltration, cm", xlab="")
mtext(side=1,"time, min", line=3)
plot(H$minutes,runoff,type="S", ylab = "runoff, cm", xlab="")
mtext(side=1,"time, min", line=3)

