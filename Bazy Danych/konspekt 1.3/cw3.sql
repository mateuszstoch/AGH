select l.member_no , l.isbn , l.fine_assessed, l.fine_assessed * 2 as 'double fine'  
from loanhist l 
where isnull(l.fine_assessed,0) > 0