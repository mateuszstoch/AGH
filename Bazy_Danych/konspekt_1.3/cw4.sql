SELECT lower(m.firstname + m.middleinitial + substring(m.lastname,2,1)) as 'e_mail' 
from [member] m 
where m.lastname  = 'Anderson'