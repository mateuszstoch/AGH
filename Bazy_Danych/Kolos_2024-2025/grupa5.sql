
--zadanie 1
select m.firstname ,m.lastname,
(select count(*) from juvenile j where j.adult_member_no = a.member_no),
(select count(*) from reservation r where r.member_no = a.member_no ),
(select count(*) from loan l where l.member_no = a.member_no )
from adult a 
join [member] m on a.member_no = m.member_no
group by m.firstname ,m.lastname, a.member_no 

--zapytania sprawdzajace
select * from juvenile j where j.adult_member_no = 1

select * from reservation r where r.member_no = 1

select * from loan l where l.member_no = 1

--zadanie 2
select top 1 t.title , 
(select top 1 l2.in_date from loanhist l2 where l2.title_no = t.title_no and year(l2.in_date) = 2001 order by 1 desc)
from title t 
join loanhist l on l.title_no = t.title_no and year(l.out_date) = 2001
group by t.title_no,t.title
order by count(*) desc

--zadanie 3
--1 sposob 
select o.OrderID,max(o.Freight),(select avg(o2.Freight) from Orders o2 where year(o2.OrderDate) = year(o.OrderDate)) from orders o 
group by o.OrderID,o.Freight,o.OrderDate
HAVING max(o.Freight) > (select avg(o2.Freight) from Orders o2 where year(o2.OrderDate) = year(o.OrderDate))

-- 2 sposob
select * from 
	(select o.OrderID,
		max(o.Freight) as maksymalna ,
		(select avg(o2.Freight) from Orders o2 where year(o2.OrderDate) = year(o.OrderDate)) as srednia 
		from orders o 
		group by o.OrderID,o.Freight,o.OrderDate) as x
where x.maksymalna  > x.srednia 
