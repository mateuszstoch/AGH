--zadanie 1
select m.firstname ,m.lastname ,j.birth_date  from juvenile j inner join [member] m on j.member_no = m.member_no 

--zadanie 2
select distinct t.title from loan l inner JOIN title t on t.title_no = l.title_no 

--zadanie 3
select l.due_date, DATEDIFF(day,l.due_date,l.in_date),l.fine_paid from loanhist l 
inner join title t on t.title_no = l.title_no 
where t.title = 'Tao Teh King' and l.due_date < l.in_date 

--zadanie 4
select r.isbn from reservation r inner join [member] m on r.member_no  = m.member_no 
where m.firstname = 'Stephen' and m.middleinitial = 'A' and m.lastname = 'Graff'