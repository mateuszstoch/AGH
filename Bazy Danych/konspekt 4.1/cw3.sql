--zadanie 1
select m.firstname ,m.lastname ,(Select count(*) from juvenile j where j.adult_member_no = a.member_no) 
from [member] m join adult a on m.member_no = a.member_no

--zadanie 2

select m.firstname ,m.lastname , m.member_no,
(Select count(*) from juvenile j where j.adult_member_no = a.member_no) ,
(Select count(*) from reservation r where r.member_no = m.member_no),
(Select count(*) from loan l where l.member_no = m.member_no)
from [member] m join adult a on m.member_no = a.member_no
group by m.firstname ,m.lastname, m.member_no, a.member_no 

--zadanie 3

select m.member_no ,m.firstname ,m.lastname ,
(Select count(*) from juvenile j where j.adult_member_no = a.member_no) as 'kids',
(Select count(*) from reservation r where r.member_no = m.member_no)+
(Select count(*) from reservation r where r.member_no in (Select j.member_no from juvenile j where j.adult_member_no = a.member_no)) as 'reservations',
(Select count(*) from loan l where l.member_no = m.member_no) + 
(Select count(*) from loan l where l.member_no in (Select j.member_no from juvenile j where j.adult_member_no = a.member_no)) as 'loans'
from [member] m join adult a on m.member_no = a.member_no
group by m.firstname ,m.lastname, m.member_no, a.member_no 

--zadanie 4
SELECT t.title ,(select count(*) from loanhist l where l.title_no = t.title_no and year(l.out_date) = 2001 ) from title t


--zadanie 5
SELECT t.title ,(select count(*) from loanhist l where l.title_no = t.title_no and year(l.out_date) = 2002 ) from title t
