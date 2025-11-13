--zadanie 1
select m.firstname, m.lastname, count(j.member_no) from adult a
join juvenile j  on a.member_no  = j.adult_member_no 
JOIN [member] m  on m.member_no = a.member_no 
where a.state ='AZ'
group by m.firstname, m.lastname, a.member_no 
having count(j.adult_member_no)>2

--zadanie 2
select m.firstname, m.lastname, count(j.member_no) from adult a
join juvenile j  on a.member_no  = j.adult_member_no 
JOIN [member] m  on m.member_no = a.member_no 
where a.state ='AZ'
group by m.firstname, m.lastname, a.member_no 
having count(j.adult_member_no)>2
union
select m.firstname, m.lastname, count(j.member_no) from adult a
join juvenile j  on a.member_no  = j.adult_member_no 
JOIN [member] m  on m.member_no = a.member_no 
where a.state ='CA'
group by m.firstname, m.lastname, a.member_no 
having count(j.adult_member_no)>3